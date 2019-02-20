/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WidgetResizer.h"
#include "Widget.h"
#include "Cylinder.h"
#include "Quad.h"
#include "vrb/ConcreteClass.h"

#include "vrb/Color.h"
#include "vrb/CreationContext.h"
#include "vrb/Matrix.h"
#include "vrb/Geometry.h"
#include "vrb/RenderState.h"
#include "vrb/SurfaceTextureFactory.h"
#include "vrb/TextureGL.h"
#include "vrb/TextureSurface.h"
#include "vrb/Toggle.h"
#include "vrb/Transform.h"
#include "vrb/Vector.h"
#include "vrb/VertexArray.h"

namespace crow {

struct ResizeBar;

typedef std::shared_ptr<ResizeBar> ResizeBarPtr;

static const float kBarSize = 0.07f;
static const float kHandleRadius = 0.08f;
static const vrb::Vector kMinResize(1.5f, 1.5f, 0.0f);
static const vrb::Vector kMaxResize(8.0f, 4.5f, 0.0f);
static vrb::Color kDefaultColor(0x2BD5D5FF);
static vrb::Color kHoverColor(0xf7ce4dff);
static vrb::Color kActiveColor(0xf7ce4dff);

enum class ResizeState {
  Default,
  Hovered,
  Active,
};


template <typename T>
static void UpdateResizeMaterial(const T& aTarget, ResizeState aState) {
  vrb::Color ambient(0.5f, 0.5f, 0.5f, 1.0f);
  vrb::Color diffuse = kDefaultColor;
  if (aState == ResizeState::Hovered) {
    diffuse = kHoverColor;
  } else if (aState == ResizeState::Active) {
    diffuse = kActiveColor;
  }

  aTarget->SetMaterial(ambient, diffuse, vrb::Color(0.0f, 0.0f, 0.0f), 0.0f);
}

struct ResizeBar {
  enum class Mode {
    Quad,
    Cylinder
  };
  static ResizeBarPtr Create(vrb::CreationContextPtr& aContext, const vrb::Vector& aCenter, const vrb::Vector& aScale, const vrb::TextureGLPtr aTexture, const ResizeBar::Mode aMode) {
    auto result = std::make_shared<ResizeBar>();
    result->center = aCenter;
    result->scale = aScale;
    vrb::Vector max(kBarSize * 0.5f, kBarSize * 0.5f, 0.0f);
    result->transform = vrb::Transform::Create(aContext);
    if (aMode == ResizeBar::Mode::Cylinder) {
      result->cylinder = Cylinder::Create(aContext, 1.0f, kBarSize);
      result->cylinder->SetLightsEnabled(false);
      result->cylinder->SetTexture(aTexture, aTexture->GetWidth(), aTexture->GetHeight());
      result->transform->AddNode(result->cylinder->GetRoot());
    } else {
      result->geometry = Quad::CreateGeometry(aContext, -max, max);
      result->geometry->GetRenderState()->SetLightsEnabled(false);
      if (aScale.y() > 0.0f) { // Rotate UVS so alpha gradient is vertical
        vrb::VertexArrayPtr array = result->geometry->GetVertexArray();
        array->SetUV(3, vrb::Vector(0.0f, 1.0f, 0));
        array->SetUV(0, vrb::Vector(1.0f, 1.0f, 0));
        array->SetUV(1, vrb::Vector(1.0f, 0.0f, 0));
        array->SetUV(2, vrb::Vector(0.0f, 0.0f, 0));
      }
      vrb::TexturePtr texture = aTexture;
      result->geometry->GetRenderState()->SetTexture(texture);
      result->transform->AddNode(result->geometry);
    }

    result->resizeState = ResizeState::Default;
    result->UpdateMaterial();
    return result;
  }

  void SetResizeState(ResizeState aState) {
    if (resizeState != aState) {
      resizeState = aState;
      UpdateMaterial();
    }
  }

  void UpdateMaterial() {
    if (cylinder) {
      UpdateResizeMaterial(cylinder, resizeState);
    } else {
      UpdateResizeMaterial(geometry->GetRenderState(), resizeState);
    }
  }

  vrb::Vector center;
  vrb::Vector scale;
  CylinderPtr cylinder;
  vrb::GeometryPtr geometry;
  vrb::TransformPtr transform;
  ResizeState resizeState;
};


struct ResizeHandle;
typedef std::shared_ptr<ResizeHandle> ResizeHandlePtr;

struct ResizeHandle {
  enum class ResizeMode {
    Vertical,
    Horizontal,
    Both
  };

  static ResizeHandlePtr Create(vrb::CreationContextPtr& aContext, const vrb::Vector& aCenter, const vrb::TextureGLPtr aTexture, ResizeMode aResizeMode, const std::vector<ResizeBarPtr>& aAttachedBars) {
    auto result = std::make_shared<ResizeHandle>();
    result->center = aCenter;
    result->resizeMode = aResizeMode;
    result->attachedBars = aAttachedBars;
    vrb::Vector max(kHandleRadius, kHandleRadius, 0.0f);
    result->geometry = Quad::CreateGeometry(aContext, -max, max);
    result->geometry->GetRenderState()->SetTexture(aTexture);
    result->transform = vrb::Transform::Create(aContext);
    result->resizeState = ResizeState ::Default;
    result->CreateOverlays(aContext);
    result->transform->AddNode(result->geometry);
    result->UpdateResizeMaterials();
    return result;
  }

  vrb::TransformPtr CreateOverlay(vrb::CreationContextPtr& aContext, const vrb::Vector& aPosition) {
    const float size = kBarSize * 0.25f;
    vrb::Vector max(size, size, 0.0f);
    vrb::GeometryPtr geometry = Quad::CreateGeometry(aContext, -max, max);
    vrb::TransformPtr transform = vrb::Transform::Create(aContext);
    overlays.push_back(geometry);
    transform->AddNode(geometry);
    const float r = kHandleRadius * 0.76f;
    transform->SetTransform(vrb::Matrix::Translation(vrb::Vector(-r + 2.0f * r * aPosition.x(), -r + 2.0f * r * aPosition.y(), 0.0f)));
    return transform;
  }

  void CreateOverlays(vrb::CreationContextPtr& aContext) {
    if (center.x() > 0.0f && center.y() != 0.5f) {
      transform->AddNode(CreateOverlay(aContext, vrb::Vector(0.0f, 0.5f, 0.0f)));
    }
    if (center.x() < 1.0f && center.y() != 0.5f) {
      transform->AddNode(CreateOverlay(aContext, vrb::Vector(1.0f, 0.5f, 0.0f)));
    }
    if (center.y() > 0.0f && center.x() != 0.5f) {
      transform->AddNode(CreateOverlay(aContext, vrb::Vector(0.5f, 0.0f, 0.0f)));
    }
    if (center.y() < 1.0f && center.x() != 0.5f) {
      transform->AddNode(CreateOverlay(aContext, vrb::Vector(0.5f, 1.0f, 0.0f)));
    }
  }

  void UpdateResizeMaterials() {
    UpdateResizeMaterial(geometry->GetRenderState(), resizeState);
    for (const vrb::GeometryPtr& overlay: overlays) {
     UpdateResizeMaterial(overlay->GetRenderState(), resizeState);
    }
  }

  void SetResizeState(ResizeState aState) {
    if (resizeState != aState) {
      resizeState = aState;
      UpdateResizeMaterials();
    }

    for (const ResizeBarPtr& bar: attachedBars) {
      bar->SetResizeState(aState);
    }
  }

  vrb::Vector center;
  ResizeMode resizeMode;
  std::vector<ResizeBarPtr> attachedBars;
  vrb::GeometryPtr geometry;
  vrb::TransformPtr transform;
  ResizeState resizeState;
  float touchRatio;
  std::vector<vrb::GeometryPtr> overlays;
};

struct WidgetResizer::State {
  vrb::CreationContextWeak context;
  Widget * widget;
  vrb::Vector min;
  vrb::Vector max;
  vrb::Vector resizeStartMin;
  vrb::Vector resizeStartMax;
  vrb::Vector currentMin;
  vrb::Vector currentMax;
  vrb::Vector pointerOffset;
  bool resizing;
  vrb::TogglePtr root;
  std::vector<ResizeHandlePtr> resizeHandles;
  std::vector<ResizeBarPtr> resizeBars;
  ResizeHandlePtr activeHandle;
  bool wasPressed;

  State()
      : widget(nullptr)
      , resizing(false)
      , wasPressed(false)
  {}

  void Initialize() {
    vrb::CreationContextPtr create = context.lock();
    if (!create) {
      return;
    }
    root = vrb::Toggle::Create(create);
    currentMin = min;
    currentMax = max;

    vrb::Vector horizontalSize(0.0f, 0.5f, 0.0f);
    vrb::Vector verticalSize(0.5f, 0.0f, 0.0f);
    vrb::TextureGLPtr barTexture = create->LoadTexture("resize_bar.png", true);
    ResizeBar::Mode mode = widget->GetCylinder() ? ResizeBar::Mode::Cylinder : ResizeBar::Mode::Quad;
    ResizeBarPtr leftTop = CreateResizeBar(vrb::Vector(0.0f, 0.75f, 0.0f), horizontalSize, barTexture, ResizeBar::Mode::Quad);
    ResizeBarPtr leftBottom = CreateResizeBar(vrb::Vector(0.0f, 0.25f, 0.0f), horizontalSize, barTexture, ResizeBar::Mode::Quad);
    ResizeBarPtr rightTop = CreateResizeBar(vrb::Vector(1.0f, 0.75f, 0.0f), horizontalSize, barTexture, ResizeBar::Mode::Quad);
    ResizeBarPtr rightBottom = CreateResizeBar(vrb::Vector(1.0f, 0.25f, 0.0f), horizontalSize, barTexture, ResizeBar::Mode::Quad);
    ResizeBarPtr topLeft = CreateResizeBar(vrb::Vector(0.25f, 1.0f, 0.0f), verticalSize, barTexture, mode);
    ResizeBarPtr topRight = CreateResizeBar(vrb::Vector(0.75f, 1.0f, 0.0f), verticalSize, barTexture, mode);
    ResizeBarPtr bottomLeft = CreateResizeBar(vrb::Vector(0.25f, 0.0f, 0.0f), verticalSize, barTexture, mode);
    ResizeBarPtr bottomRight = CreateResizeBar(vrb::Vector(0.75f, 0.0f, 0.0f), verticalSize, barTexture, mode);

    vrb::TextureGLPtr handleTexture = create->LoadTexture("resize_handle.png", true);

    CreateResizeHandle(vrb::Vector(0.0f, 1.0f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Both, {leftTop, topLeft});
    CreateResizeHandle(vrb::Vector(1.0f, 1.0f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Both, {rightTop, topRight});
    //CreateResizeHandle(vrb::Vector(0.0f, 0.0f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Both, {leftBottom, bottomLeft});
    //CreateResizeHandle(vrb::Vector(1.0f, 0.0f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Both, {rightBottom, bottomRight}, 1.0f);
    CreateResizeHandle(vrb::Vector(0.5f, 1.0f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Horizontal, {topLeft, topRight});
    //CreateResizeHandle(vrb::Vector(0.5f, 0.0f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Horizontal, {bottomLeft, bottomRight});
    CreateResizeHandle(vrb::Vector(0.0f, 0.5f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Vertical, {leftTop, leftBottom});
    CreateResizeHandle(vrb::Vector(1.0f, 0.5f, 0.0f), handleTexture, ResizeHandle::ResizeMode::Vertical, {rightTop, rightBottom});

    Layout();
  }

  ResizeBarPtr CreateResizeBar(const vrb::Vector& aCenter, vrb::Vector aScale, const vrb::TextureGLPtr aTexture, const ResizeBar::Mode aMode) {
    vrb::CreationContextPtr create = context.lock();
    if (!create) {
      return nullptr;
    }
    ResizeBarPtr result = ResizeBar::Create(create, aCenter, aScale, aTexture, aMode);
    resizeBars.push_back(result);
    root->AddNode(result->transform);
    return result;
  }

  ResizeHandlePtr CreateResizeHandle(const vrb::Vector& aCenter, const vrb::TextureGLPtr aTexture, ResizeHandle::ResizeMode aResizeMode, const std::vector<ResizeBarPtr>& aBars, const float aTouchRatio = 2.0f) {
    vrb::CreationContextPtr create = context.lock();
    if (!create) {
      return nullptr;
    }
    ResizeHandlePtr result = ResizeHandle::Create(create, aCenter, aTexture, aResizeMode, aBars);
    result->touchRatio = aTouchRatio;
    resizeHandles.push_back(result);
    root->InsertNode(result->transform, 0);
    return result;
  }

  float WorldWidth() const {
    return max.x() - min.x();
  }

  float WorldHeight() const {
    return max.y() - min.y();
  }

  vrb::Vector ProjectPoint(const vrb::Vector& aWorldPoint) const {
    vrb::Matrix modelView = widget->GetTransformNode()->GetWorldTransform().AfineInverse();
    if (widget->GetCylinder()) {
      // Map the position in the cylinder to the position it would have on a quad
      const float radius = widget->GetCylinder()->GetTransformNode()->GetTransform().GetScale().x();
      float cosAngle = fminf(1.0f, fabsf(aWorldPoint.x()) / radius);
      const float sign = aWorldPoint.x() > 0 ? 1.0f : -1.0f;
      const float angle = acosf(cosAngle);
      const float surfaceArc = (float)M_PI - angle * 2.0f;
      const float projectedWidth = WorldWidth() * surfaceArc / widget->GetCylinder()->GetCylinderTheta();
      vrb::Vector point(projectedWidth * 0.5f * sign, aWorldPoint.y(), aWorldPoint.z());
      vrb::Vector result = modelView.MultiplyPosition(point);
      result.z() = 0.0f;
      return result;
    } else {
      return modelView.MultiplyPosition(aWorldPoint);
    }
  }

  void LayoutQuad() {
    const float width = WorldWidth();
    const float height = WorldHeight();

    for (ResizeBarPtr& bar: resizeBars) {
      float targetWidth = bar->scale.x() > 0.0f ? (bar->scale.x() * fabsf(width)) + kBarSize * 0.5f : kBarSize;
      float targetHeight = bar->scale.y() > 0.0f ? (bar->scale.y() * fabs(height)) - kBarSize * 0.5f : kBarSize;
      vrb::Matrix matrix = vrb::Matrix::Position(vrb::Vector(min.x() + width * bar->center.x(), min.y() + height * bar->center.y(), 0.005f));
      matrix.ScaleInPlace(vrb::Vector(targetWidth / kBarSize, targetHeight / kBarSize, 1.0f));
      bar->transform->SetTransform(matrix);
    }

    for (ResizeHandlePtr& handle: resizeHandles) {
      vrb::Matrix matrix = vrb::Matrix::Position(vrb::Vector(min.x() + width * handle->center.x(), min.y() + height * handle->center.y(), 0.006f));
      handle->transform->SetTransform(matrix);
    }
  }

  void LayoutCylinder() {
    const float width = currentMax.x() - currentMin.x();
    const float height = currentMax.y() - currentMin.y();
    const float sx = width / WorldWidth();
    const float radius = widget->GetCylinder()->GetTransformNode()->GetTransform().GetScale().x() - kBarSize * 0.5f;
    const float theta = widget->GetCylinder()->GetCylinderTheta() * sx;
    int32_t textureWidth, textureHeight;
    widget->GetCylinder()->GetTextureSize(textureWidth, textureHeight);
    vrb::Matrix modelView = widget->GetTransformNode()->GetWorldTransform().AfineInverse();

    for (ResizeBarPtr& bar: resizeBars) {
      float targetWidth = bar->scale.x() > 0.0f ? (bar->scale.x() * fabsf(width)) + kBarSize * 0.5f : kBarSize;
      float targetHeight = bar->scale.y() > 0.0f ? (bar->scale.y() * fabs(height)) - kBarSize * 0.5f : kBarSize;
      const float pointerAngle = (float)M_PI * 0.5f + theta * 0.5f - theta * bar->center.x();
      vrb::Matrix rotation = vrb::Matrix::Rotation(vrb::Vector(-cosf(pointerAngle), 0.0f, sinf(pointerAngle)));
      if (bar->cylinder) {
        bar->cylinder->SetCylinderTheta(theta * 0.5f);
        vrb::Matrix translation = vrb::Matrix::Position(vrb::Vector(0.0f, min.y() + height * bar->center.y(), radius));
        vrb::Matrix scale = vrb::Matrix::Identity();
        scale.ScaleInPlace(vrb::Vector(radius, 1.0f, radius));
        bar->transform->SetTransform(translation.PostMultiply(scale).PostMultiply(rotation));
      } else {
        vrb::Matrix translation = vrb::Matrix::Position(vrb::Vector(radius * cosf(pointerAngle),
                                                        min.y() + height * bar->center.y(),
                                                        radius - radius * sinf(pointerAngle)));
        vrb::Matrix scale = vrb::Matrix::Identity();
        scale.ScaleInPlace(vrb::Vector(targetWidth / kBarSize, targetHeight / kBarSize, 1.0f));
        bar->transform->SetTransform(translation.PostMultiply(scale).PostMultiply(rotation));
      }
    }

    for (ResizeHandlePtr& handle: resizeHandles) {
      const float pointerAngle = (float)M_PI * 0.5f + theta * 0.5f - theta * handle->center.x();
      vrb::Matrix translation = vrb::Matrix::Position(vrb::Vector(radius * cosf(pointerAngle),
                                                      min.y() + height * handle->center.y(),
                                                      radius - radius * sinf(pointerAngle)));
      vrb::Matrix rotation = vrb::Matrix::Rotation(vrb::Vector(-cosf(pointerAngle), 0.0f, sinf(pointerAngle)));
      handle->transform->SetTransform(translation.PostMultiply(rotation));
    }
  }

  void Layout() {
    if (widget->GetCylinder()) {
      LayoutCylinder();
    } else {
      LayoutQuad();
    }
  }

  ResizeHandlePtr GetIntersectingHandler(const vrb::Vector& point) {
    for (const ResizeHandlePtr& handle: resizeHandles) {
      vrb::Vector worldCenter(min.x() + WorldWidth() * handle->center.x(), min.y() + WorldHeight() * handle->center.y(), 0.0f);
      float distance = (point - worldCenter).Magnitude();
      if (distance < kHandleRadius * handle->touchRatio) {
        return handle;
      }
    }
    return nullptr;
  }

  void HandleResize(const vrb::Vector& aPoint) {
    if (!activeHandle) {
      return;
    }

    const vrb::Vector point = aPoint - pointerOffset;
    float originalWidth = fabsf(resizeStartMax.x() - resizeStartMin.x());
    float originalHeight = fabsf(resizeStartMax.y() - resizeStartMin.y());
    float originalAspect = originalWidth / originalHeight;

    float width = fabsf(point.x()) * 2.0f;
    float height = fabsf(point.y() - min.y());

    // Calculate resize based on resize mode
    bool keepAspect = false;
    if (activeHandle->resizeMode == ResizeHandle::ResizeMode::Vertical) {
      height = originalHeight;
    } else if (activeHandle->resizeMode == ResizeHandle::ResizeMode::Horizontal) {
      width = originalWidth;
    } else {
      width = fmaxf(width, height * originalAspect);
      height = width / originalAspect;
      keepAspect = true;
    }

    // Clamp to max and min resize sizes
    width = fmaxf(fminf(width, kMaxResize.x()), kMinResize.x());
    height = fmaxf(fminf(height, kMaxResize.y()), kMinResize.y());
    if (keepAspect) {
      height = width / originalAspect;
    }

    currentMin = vrb::Vector(-width * 0.5f, -height * 0.5f, 0.0f);
    currentMax = vrb::Vector(width * 0.5f, height * 0.5f, 0.0f);

    // Reset world min and max points with the new resize values
    if (!widget->GetCylinder() || keepAspect) {
      min = currentMin;
      max = currentMax;
    }

    Layout();
  }
};

WidgetResizerPtr
WidgetResizer::Create(vrb::CreationContextPtr& aContext, Widget * aWidget) {
  WidgetResizerPtr result = std::make_shared<vrb::ConcreteClass<WidgetResizer, WidgetResizer::State> >(aContext);
  aWidget->GetWidgetMinAndMax(result->m.min, result->m.max);
  result->m.widget = aWidget;
  result->m.Initialize();
  return result;
}


vrb::NodePtr
WidgetResizer::GetRoot() const {
  return m.root;
}

void
WidgetResizer::SetSize(const vrb::Vector& aMin, const vrb::Vector& aMax) {
  m.min = aMin;
  m.max = aMax;
  m.currentMin = aMin;
  m.currentMax = aMax;
  m.Layout();
}

void
WidgetResizer::ToggleVisible(bool aVisible) {
  m.root->ToggleAll(aVisible);
}

bool
WidgetResizer::TestIntersection(const vrb::Vector& aWorldPoint) const {
  if (m.activeHandle) {
    return true;
  }
  const vrb::Vector point = m.ProjectPoint(aWorldPoint);
  if (m.widget->GetCylinder()) {
    //point =
  }
  vrb::Vector extraMin = vrb::Vector(m.min.x() - kBarSize * 0.5f, m.min.y() - kBarSize * 0.5f, 0.0f);
  vrb::Vector extraMax = vrb::Vector(m.max.x() + kBarSize * 0.5f, m.max.y() + kBarSize * 0.5f, 0.0f);

  if ((point.x() >= extraMin.x()) && (point.y() >= extraMin.y()) &&(point.z() >= (extraMin.z() - 0.1f)) &&
      (point.x() <= extraMax.x()) && (point.y() <= extraMax.y()) &&(point.z() <= (extraMax.z() + 0.1f))) {

    return true;
  }

  return m.GetIntersectingHandler(point).get() != nullptr;
}

void
WidgetResizer::HandleResizeGestures(const vrb::Vector& aWorldPoint, bool aPressed, bool& aResized, bool &aResizeEnded) {
  const vrb::Vector point = m.ProjectPoint(aWorldPoint);
  for (const ResizeHandlePtr& handle: m.resizeHandles) {
    handle->SetResizeState(ResizeState::Default);
  }
  aResized = false;
  aResizeEnded = false;

  if (aPressed && !m.wasPressed) {
    // Handle resize handle click
    m.activeHandle = m.GetIntersectingHandler(point);
    if (m.activeHandle) {
      m.resizeStartMin = m.min;
      m.resizeStartMax = m.max;
      m.currentMin = m.min;
      m.currentMax = m.max;
      m.activeHandle->SetResizeState(ResizeState::Active);
      vrb::Vector center(m.min.x() + m.WorldWidth() * m.activeHandle->center.x(),
                         m.min.y() + m.WorldHeight() * m.activeHandle->center.y(), 0.0f);
      m.pointerOffset = point - center;
    }
  } else if (!aPressed && m.wasPressed) {
    // Handle resize handle unclick
    if (m.activeHandle) {
      m.activeHandle->SetResizeState(ResizeState::Hovered);
      m.min = m.currentMin;
      m.max = m.currentMax;
      aResizeEnded = true;
    }
    m.activeHandle.reset();
  } else if (aPressed && m.activeHandle) {
    // Handle resize gesture
    m.activeHandle->SetResizeState(ResizeState::Active);
    m.HandleResize(point);
    aResized = true;
  } else if (!aPressed) {
    // Handle hover
    ResizeHandlePtr handle = m.GetIntersectingHandler(point);
    if (handle) {
      handle->SetResizeState(ResizeState::Hovered);
    }
  }

  m.wasPressed = aPressed;
}

void
WidgetResizer::HoverExitResize() {
  for (const ResizeHandlePtr& handle: m.resizeHandles) {
    handle->SetResizeState(ResizeState::Default);
  }
  m.wasPressed = false;
}

const vrb::Vector&
WidgetResizer::GetCurrentMin() const {
  return m.min;
}

const vrb::Vector&
WidgetResizer::GetCurrentMax() const {
  return m.max;
}


WidgetResizer::WidgetResizer(State& aState, vrb::CreationContextPtr& aContext) : m(aState) {
  m.context = aContext;
}

WidgetResizer::~WidgetResizer() {}

} // namespace crow

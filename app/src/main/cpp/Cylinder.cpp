/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Cylinder.h"
#include "Quad.h"
#include "VRLayer.h"
#include "VRLayerNode.h"
#include "vrb/ConcreteClass.h"

#include "vrb/Color.h"
#include "vrb/CreationContext.h"
#include "vrb/Matrix.h"
#include "vrb/Geometry.h"
#include "vrb/RenderState.h"
#include "vrb/SurfaceTextureFactory.h"
#include "vrb/TextureSurface.h"
#include "vrb/Toggle.h"
#include "vrb/Transform.h"
#include "vrb/Vector.h"
#include "vrb/VertexArray.h"

namespace crow {

struct Cylinder::State {
  vrb::CreationContextWeak context;
  VRLayerCylinderPtr layer;
  VRLayerNodePtr layerNode;
  int32_t textureWidth;
  int32_t textureHeight;
  vrb::TogglePtr root;
  vrb::TransformPtr transform;
  vrb::GeometryPtr geometry;
  vrb::Vector worldMin;
  vrb::Vector worldMax;

  State()
      : textureWidth(0)
      , textureHeight(0)
      , worldMin(0.0f, 0.0f, 0.0f)
      , worldMax(0.0f, 0.0f, 0.0f)
  {}

  void Initialize() {
    vrb::CreationContextPtr create = context.lock();
    transform = vrb::Transform::Create(create);
    if (layer) {
      textureWidth = layer->GetWidth();
      textureHeight = layer->GetHeight();
      layer->SetWorldSize(GetWorldWidth(), GetWorldHeight());
      layerNode = VRLayerNode::Create(create, layer);
      transform->AddNode(layerNode);
    } else {
      // TODO: Create cylinder geometry
      VRB_LOG("Cylinder geometry not implemented yet");
    }
    root = vrb::Toggle::Create(create);
    root->AddNode(transform);
  }

  float GetWorldWidth() const {
    return worldMax.x() - worldMin.x();
  }

  float GetWorldHeight() const {
    return worldMax.y() - worldMin.y();
  }

  float GetTheta() const {
    const float width = textureWidth / layer->GetPixelDensity();
    return (float)M_PI * width / (layer->GetCylinderDensity() * 0.5f);
  }
};

CylinderPtr
Cylinder::Create(vrb::CreationContextPtr aContext, const vrb::Vector& aMin, const vrb::Vector& aMax, const VRLayerCylinderPtr& aLayer) {
  CylinderPtr result = std::make_shared<vrb::ConcreteClass<Cylinder, Cylinder::State> >(aContext);
  result->m.worldMin = aMin;
  result->m.worldMax = aMax;
  result->m.layer = aLayer;
  result->m.Initialize();
  return result;
}

CylinderPtr
Cylinder::Create(vrb::CreationContextPtr aContext, const float aWorldWidth, const float aWorldHeight, const VRLayerCylinderPtr& aLayer) {
  CylinderPtr result = std::make_shared<vrb::ConcreteClass<Cylinder, Cylinder::State> >(aContext);
  result->m.worldMin = vrb::Vector(-aWorldWidth * 0.5f, -aWorldHeight * 0.5f, 0.0f);
  result->m.worldMax = vrb::Vector(aWorldWidth * 0.5f, aWorldHeight * 0.5f, 0.0f);
  result->m.layer = aLayer;
  result->m.Initialize();
  return result;
}

void
Cylinder::GetTextureSize(int32_t& aWidth, int32_t& aHeight) const {
  aWidth = m.textureWidth;
  aHeight = m.textureHeight;
}

void
Cylinder::SetTextureSize(int32_t aWidth, int32_t aHeight) {
  m.textureWidth = aWidth;
  m.textureHeight = aHeight;
  if (m.layer) {
    m.layer->Resize(aWidth, aHeight);
  }
}

void
Cylinder::GetWorldMinAndMax(vrb::Vector& aMin, vrb::Vector& aMax) const {
  aMin = m.worldMin;
  aMax = m.worldMax;
}

const vrb::Vector&
Cylinder::GetWorldMin() const {
  return m.worldMin;
}

const vrb::Vector&
Cylinder::GetWorldMax() const {
  return m.worldMax;
}

float
Cylinder::GetWorldWidth() const {
  return  m.GetWorldWidth();
}

float
Cylinder::GetWorldHeight() const {
  return m.GetWorldHeight();
}

void
Cylinder::GetWorldSize(float& aWidth, float& aHeight) const {
  aWidth = m.worldMax.x() - m.worldMin.x();
  aHeight = m.worldMax.y() - m.worldMin.y();
}

float
Cylinder::GetCylinderDensity() const {
  return m.layer->GetCylinderDensity();
}

float
Cylinder::GetCylinderRadius() const {
  return m.layer->GetRadius();
}

void
Cylinder::SetCylinderDensity(const float aDensity) {
  if (m.layer) {
    m.layer->SetCylinderDensity(aDensity);
  }
}

void
Cylinder::SetCylinderRadius(const float aRadius) {
  if (m.layer) {
    m.layer->SetRadius(aRadius);
  }
}

void
Cylinder::SetWorldSize(const float aWidth, const float aHeight) const {
  vrb::Vector min = vrb::Vector(-aWidth * 0.5f, -aHeight * 0.5f, 0.0f);
  vrb::Vector max = vrb::Vector(aWidth * 0.5f, aHeight * 0.5f, 0.0f);
  SetWorldSize(min, max);
}

void
Cylinder::SetWorldSize(const vrb::Vector& aMin, const vrb::Vector& aMax) const {
  if (m.worldMin == aMin && m.worldMax == aMax) {
    return;
  }
  m.worldMin = aMin;
  m.worldMax = aMax;

  if (m.layer) {
    m.layer->SetWorldSize(GetWorldWidth(), GetWorldHeight());
  }
}

void
Cylinder::SetTintColor(const vrb::Color& aColor) {
  if (m.layer) {
    m.layer->SetTintColor(aColor);
  } else if (m.geometry && m.geometry->GetRenderState()) {
    m.geometry->GetRenderState()->SetTintColor(aColor);
  }
}

vrb::Vector
Cylinder::GetCenterNormal() const {
  const vrb::Vector bottomRight(m.worldMax.x(), m.worldMin.y(), m.worldMin.z());
  return (bottomRight - m.worldMin).Cross(m.worldMax - m.worldMin).Normalize();
}

vrb::NodePtr
Cylinder::GetRoot() const {
  return m.root;
}

VRLayerCylinderPtr
Cylinder::GetLayer() const {
  return m.layer;
}

vrb::TransformPtr
Cylinder::GetTransformNode() const {
  return m.transform;
}

void
Cylinder::SetTransform(const vrb::Matrix& aTransform) {
  m.transform->SetTransform(aTransform);
}

static const float kEpsilon = 0.00000001f;

bool
Cylinder::TestIntersection(const vrb::Vector& aStartPoint, const vrb::Vector& aDirection, vrb::Vector& aResult, vrb::Vector& aNormal, bool aClamp, bool& aIsInside, float& aDistance) const {
  aDistance = -1.0f;
  if (!m.root->IsEnabled(*m.transform)) {
    return false;
  }

  vrb::Matrix worldTransform = m.transform->GetWorldTransform();
  vrb::Matrix modelView = worldTransform.AfineInverse();
  vrb::Vector start = modelView.MultiplyPosition(aStartPoint);
  vrb::Vector direction = modelView.MultiplyDirection(aDirection);
  start = start - direction * 1000.0f;

  const float radius = this->GetCylinderRadius();
  const float height = radius * 2.0f;
  const vrb::Vector A(0.0f, -height * 0.5f, 0.0f); // Cylinder bottom center
  const vrb::Vector B(0.0f, height * 0.5f, 0.0f); // Cylinder top center

  const vrb::Vector AB = B - A;
  const vrb::Vector AO = start - A;
  const vrb::Vector AOxAB = AO.Cross(AB);
  const vrb::Vector VxAB  = direction.Cross(AB);
  // Solve quadratic formula
  const float ab2 = AB.Dot(AB);
  const float a = VxAB.Dot(VxAB);
  const float b = 2 * VxAB.Dot(AOxAB);
  const float c = AOxAB.Dot(AOxAB) - (radius * radius * ab2);
  const float d = b * b - 4 * a * c;
  if (d < 0) {
    return false;
  }
  double time = (-b + sqrt(d)) / (2 * a);
  if (time < 0) {
    return false;
  }

  const vrb::Vector intersection = start + direction * time; // intersection point
  const vrb::Vector projection = A + AB * (AB.Dot(intersection - A) / ab2); // intersection projected onto cylinder axis

  // Height test
  if ((projection - A).Magnitude() + (B - projection).Magnitude() > AB.Magnitude()) {
    return false;
  }

  // Normal Test
  const vrb::Vector normal = (projection - intersection).Normalize();
  if (normal.z() < 0) {
    // Ignore cylinder side not facing the user
    return false;
  }

  // Cylinder theta angle test
  const float maxTheta = m.GetTheta();
  const float hitTheta = (float)M_PI - acosf(fabsf(intersection.x()) / radius) * 2.0f;
  aIsInside = hitTheta <= maxTheta && fabs(intersection.y()) <= radius;

  vrb::Vector result = intersection;
  // Clamp to keep pointer in cylinder surface.
  if (aClamp && !aIsInside) {
    const float maxX = radius * cosf(0.5f * ((float)M_PI - maxTheta));
    const float minX = -maxX;
    if (result.x() > maxX) { result.x() = maxX; }
    else if (result.x() < minX) { result.x() = minX; }

    if (result.y() > radius) { result.y() = radius; }
    else if (result.y() < radius) { result.y() = -radius; }
  }

  aResult = worldTransform.MultiplyPosition(intersection);
  aNormal = worldTransform.MultiplyDirection(normal);
  aDistance = (aResult - aStartPoint).Magnitude();

  return true;
}

void
Cylinder::ConvertToQuadCoordinates(const vrb::Vector& point, float& aX, float& aY, bool aClamp) const {
  const vrb::Vector intersection = m.transform->GetWorldTransform().AfineInverse().MultiplyPosition(point);
  const float radius = GetCylinderRadius();
  float ratioY;
  if (intersection.y() > 0.0f) {
    ratioY = 0.5f - 0.5f * intersection.y() / radius;
  } else {
    ratioY = 0.5f + 0.5f * fabsf(intersection.y()) / radius;
  }

  const float hitTheta = (float)M_PI - acosf(fabsf(intersection.x()) / radius) * 2.0f;
  const float maxTheta = m.GetTheta();
  float ratioTheta = hitTheta / maxTheta * 0.5f;
  float ratioX;
  if (intersection.x() > 0.0f) {
    ratioX = 0.5f + ratioTheta;
  } else {
    ratioX = 0.5f - ratioTheta;
  }

  if (aClamp) {
    if (ratioY > 1.0f) {
      ratioY = 1.0f;
    }
    if (ratioY < 0.0f) {
      ratioY = 0.0f;
    }
    if (ratioX > 1.0f) {
      ratioX = 1.0f;
    }
    if (ratioX < 0.0f) {
      ratioX = 0.0f;
    }

    aX = ratioX * m.textureWidth;
    aY = ratioY * m.textureHeight;
  }
}

float Cylinder::DistanceToBackPlane(const vrb::Vector &aStartPoint, const vrb::Vector &aDirection) const {
  float result = -1.0f;
  if (!m.root->IsEnabled(*m.transform)) {
    return result;
  }
  vrb::Matrix worldTransform = m.transform->GetWorldTransform();
  vrb::Matrix modelView = worldTransform.AfineInverse();
  vrb::Vector point = modelView.MultiplyPosition(aStartPoint);
  vrb::Vector direction = modelView.MultiplyDirection(aDirection);

  const vrb::Vector max(1.0f, 1.0f, -1.0f);
  const vrb::Vector min(-1.0f, -1.0f, -1.0f);
  const vrb::Vector bottomRight(max.x(), min.y(), min.z());
  vrb::Vector normal =  (bottomRight - min).Cross(max - min).Normalize();
  const float dotNormals = direction.Dot(normal);
  if (dotNormals > -kEpsilon) {
    // Not pointed at the plane
    return result;
  }

  const float dotV = (min - point).Dot(normal);

  if ((dotV < kEpsilon) && (dotV > -kEpsilon)) {
    return result;
  }

  const float length = dotV / dotNormals;
  vrb::Vector intersection = point + (direction * length);

  vrb::Vector worldPoint = worldTransform.MultiplyPosition(intersection);
  result = (worldPoint - aStartPoint).Magnitude();

  return result;
}

Cylinder::Cylinder(State& aState, vrb::CreationContextPtr& aContext) : m(aState) {
  m.context = aContext;
}

Cylinder::~Cylinder() {}

} // namespace crow

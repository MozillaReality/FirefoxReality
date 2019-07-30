/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WidgetMover.h"
#include "Widget.h"
#include "WidgetPlacement.h"
#include "VRBrowser.h"
#include "vrb/ConcreteClass.h"
#include "vrb/Matrix.h"

namespace crow {

// Should match the values defined in WidgetManagerDelegate.WidgetMoveBehaviourFlags
enum class WidgetMoveBehaviour {
  GENERAL = 0,
  KEYBOARD = 1
};

struct WidgetMover::State {
  WidgetPtr widget;
  int attachedController;
  vrb::Vector initialPoint;
  vrb::Matrix initialTransform;
  WidgetPlacementPtr initialPlacement;
  WidgetPlacementPtr movePlacement;
  WidgetMoveBehaviour moveBehaviour;
  vrb::Vector endDelta;
  float endRotation;

  State()
      : widget(nullptr)
      , attachedController(-1)
      , moveBehaviour(WidgetMoveBehaviour::GENERAL)
      , endRotation(0)
  {}


  WidgetPlacementPtr HandleKeyboardMove(const vrb::Vector& aDelta) {
    float x = initialPlacement->translation.x() * WidgetPlacement::WORLD_DPI_RATIO;
    float y = initialPlacement->translation.y() * WidgetPlacement::WORLD_DPI_RATIO;
    const float maxX = 3.0f;
    const float minX = -2.0f;
    const float maxY = 2.0f;
    const float minY = -1.0f;
    float x = initialPlacement->translation.x() * WidgetPlacement::kWorldDPIRatio;
    float y = initialPlacement->translation.y() * WidgetPlacement::kWorldDPIRatio;
    const float maxAngle = -35.0f * (float)M_PI / 180.0f;
    const float angleStartY = 0.8f;
    const float minZ = -2.5f;
    const float maxZ = -3.2f;
    const float thresholdZ = 1.45f;
    x += aDelta.x();
    y += aDelta.y();

    x = fmin(x, maxX);
    x = fmax(x, minX);
    y = fmin(y, maxY);
    y = fmax(y, minY);

    movePlacement->translation.x() = x / WidgetPlacement::WORLD_DPI_RATIO;
    movePlacement->translation.y() = y / WidgetPlacement::WORLD_DPI_RATIO;

    float angle = 0.0f;
    if (y < angleStartY) {
      const float t = 1.0f - (y - minY) / (angleStartY - minY);
      angle = t * maxAngle;
    }

    float t = 0.0f;
    if (y > 1.45f) {
      t = 1.0f;
    } else {
      t = (y - minY) / (1.45f - minY);
    }

    movePlacement->translation.z() = (minZ + t * (maxZ - minZ)) / WidgetPlacement::WORLD_DPI_RATIO;
    movePlacement->rotation = angle;
    endDelta = movePlacement->translation - initialPlacement->translation;
    endRotation = angle;

    return movePlacement;
  }
};

WidgetMoverPtr
WidgetMover::Create() {
  return std::make_shared<vrb::ConcreteClass<WidgetMover, WidgetMover::State> >();
}

bool
WidgetMover::IsMoving(const int aControllerIndex) const {
  return m.widget != nullptr && m.attachedController == aControllerIndex;
}

WidgetPlacementPtr
WidgetMover::HandleMove(const vrb::Vector& aStart, const vrb::Vector& aDirection) {
  float hitDistance = -1;
  vrb::Vector hitPoint;
  vrb::Vector hitNormal;
  bool isInWidget = false;
  m.widget->TestControllerIntersection(aStart, aDirection, hitPoint, hitNormal, isInWidget, hitDistance);
  if (hitDistance < 0) {
    return nullptr;
  };


  const vrb::Vector delta = hitPoint - m.initialPoint;

  if (m.moveBehaviour == WidgetMoveBehaviour::KEYBOARD) {
    return m.HandleKeyboardMove(delta);
  } else {
    // General case
    vrb::Matrix updatedTransform = m.initialTransform.Translate(vrb::Vector(delta.x(), delta.y(), 0.0f));
    m.widget->SetTransform(updatedTransform);
    m.endDelta.x() = delta.x() / WidgetPlacement::WORLD_DPI_RATIO;
    m.endDelta.y() = delta.y() / WidgetPlacement::WORLD_DPI_RATIO;
    m.endDelta.z() = 0.0f;
    m.endRotation = 0.0f;
    return nullptr;
  }
}

void
WidgetMover::StartMoving(const WidgetPtr& aWidget, const int32_t aMoveBehaviour, const int32_t aControllerIndex, const vrb::Vector& hitPoint) {
  m.widget = aWidget;
  m.attachedController = aControllerIndex;
  m.initialTransform = aWidget->GetTransform();
  m.initialPoint = hitPoint;
  m.initialPlacement = aWidget->GetPlacement();
  m.movePlacement = WidgetPlacement::Create(*m.initialPlacement);
  m.moveBehaviour = (WidgetMoveBehaviour) aMoveBehaviour;
}

void
WidgetMover::EndMoving() {
  m.attachedController = -1;
  if (m.widget) {
    VRBrowser::HandleMoveEnd(m.widget->GetHandle(), m.endDelta.x(), m.endDelta.y(), m.endDelta.z(), m.endRotation);
  }
  m.widget = nullptr;
}

WidgetPtr
WidgetMover::GetWidget() const {
  return m.widget;
}

WidgetMover::WidgetMover(State& aState) : m(aState) {
}

} // namespace crow

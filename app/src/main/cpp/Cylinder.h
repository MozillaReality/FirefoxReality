/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VRBROWSER_CYLINDER_DOT_H
#define VRBROWSER_CYLINDER_DOT_H

#include "vrb/Forward.h"
#include "vrb/MacroUtils.h"
#include "Device.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace crow {


class VRLayerCylinder;
typedef std::shared_ptr<VRLayerCylinder> VRLayerCylinderPtr;

class Cylinder;
typedef std::shared_ptr<Cylinder> CylinderPtr;

class Cylinder {
public:
  static CylinderPtr Create(vrb::CreationContextPtr aContext, const vrb::Vector& aMin, const vrb::Vector& aMax, const VRLayerCylinderPtr& aLayer = nullptr);
  static CylinderPtr Create(vrb::CreationContextPtr aContext, const float aWorldWidth, const float aWorldHeight, const VRLayerCylinderPtr& aLayer = nullptr);
  void GetTextureSize(int32_t& aWidth, int32_t& aHeight) const;
  void SetTextureSize(int32_t aWidth, int32_t aHeight);
  void GetWorldMinAndMax(vrb::Vector& aMin, vrb::Vector& aMax) const;
  const vrb::Vector& GetWorldMin() const;
  const vrb::Vector& GetWorldMax() const;
  float GetWorldWidth() const;
  float GetWorldHeight() const;
  void GetWorldSize(float& aWidth, float& aHeight) const;
  float GetCylinderDensity() const;
  float GetCylinderRadius() const;
  void SetCylinderDensity(const float aDensity);
  void SetCylinderRadius(const float aRadius);
  void SetWorldSize(const float aWidth, const float aHeight) const;
  void SetWorldSize(const vrb::Vector& aMin, const vrb::Vector& aMax) const;
  void SetTintColor(const vrb::Color& aColor);
  vrb::Vector GetCenterNormal() const;
  vrb::NodePtr GetRoot() const;
  VRLayerCylinderPtr GetLayer() const;
  vrb::TransformPtr GetTransformNode() const;
  void SetTransform(const vrb::Matrix& aTransform);
  bool TestIntersection(const vrb::Vector& aStartPoint, const vrb::Vector& aDirection, vrb::Vector& aResult, vrb::Vector& aNormal, bool aClamp, bool& aIsInside, float& aDistance) const;
  void ConvertToQuadCoordinates(const vrb::Vector& point, float& aX, float& aY, bool aClamp) const;
  float DistanceToBackPlane(const vrb::Vector& aStartPoint, const vrb::Vector& aDirection) const;

  struct State;
  Cylinder(State& aState, vrb::CreationContextPtr& aContext);
  ~Cylinder();
private:
  State& m;
  Cylinder() = delete;
  VRB_NO_DEFAULTS(Cylinder)
};

} // namespace crow

#endif // VRBROWSER_CYLINDER_DOT_H

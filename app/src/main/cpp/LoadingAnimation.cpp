/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "LoadingAnimation.h"
#include "vrb/ConcreteClass.h"
#include "vrb/CreationContext.h"
#include "vrb/Matrix.h"
#include "vrb/ModelLoaderAndroid.h"
#include "vrb/Light.h"
#include "vrb/TextureGL.h"
#include "vrb/Toggle.h"
#include "vrb/Transform.h"
#include "vrb/RenderContext.h"

#include "Quad.h"

namespace crow {

static const float scale = 0.05f;

struct LoadingAnimation::State {
  vrb::CreationContextWeak context;
  vrb::TextureGLPtr texture;
  vrb::TogglePtr root;
  vrb::TransformPtr fox;
  vrb::TransformPtr globe;
  float globe_rotation;
  double timeStamp;

  State()
      : timeStamp(0), globe_rotation(0.0f)
  {}

  void Initialize() {
    vrb::CreationContextPtr create = context.lock();
    root = vrb::Toggle::Create(create);
    vrb::LightPtr light = vrb::Light::Create(create);
    root->AddLight(light);
  }
};


void
LoadingAnimation::LoadModels(const vrb::ModelLoaderAndroidPtr& aLoader) {
  if (m.fox && m.globe) {
    return;
  }
  vrb::CreationContextPtr ctx = m.context.lock();
  m.fox = vrb::Transform::Create(ctx);
  m.globe = vrb::Transform::Create(ctx);
  m.root->AddNode(m.fox);
  m.root->AddNode(m.globe);
  aLoader->LoadModel("spinner/fxr_spinner_fox.obj", m.fox);
  aLoader->LoadModel("spinner/fxr_spinner_globe.obj", m.globe);

  vrb::Matrix foxTransform = vrb::Matrix::Identity();
  foxTransform.ScaleInPlace(vrb::Vector(scale, scale, scale));
  foxTransform.PreMultiplyInPlace(vrb::Matrix::Position(vrb::Vector(0.0f, 0.0f, -1.5f)));
  m.fox->SetTransform(foxTransform);
}

void
LoadingAnimation::Update(vrb::RenderContextPtr aContext) {
  const double ctime = aContext->GetTimestamp();

  if (m.timeStamp <= 0.0) {
    m.timeStamp = ctime;
    return;
  }

  const double delta = ctime - m.timeStamp;
  m.timeStamp = ctime;

  if (!m.fox || !m.globe) {
    return;
  }
  m.globe_rotation += 0.5f * delta;
  const float max = 2.0f * (float)M_PI;
  if (m.globe_rotation > max) {
     m.globe_rotation -= max;
  }

  vrb::Matrix globeTransform = vrb::Matrix::Identity();
  globeTransform.ScaleInPlace(vrb::Vector(scale, scale, scale));
  globeTransform.PreMultiplyInPlace(vrb::Matrix::Rotation(vrb::Vector(0.0f, 1.0f, 0.0f), m.globe_rotation));
  globeTransform.PreMultiplyInPlace(vrb::Matrix::Position(vrb::Vector(0.0f, 0.0f, -1.5f)));
  m.globe->SetTransform(globeTransform);
}

vrb::NodePtr
LoadingAnimation::GetRoot() const {
  return m.root;
}

LoadingAnimationPtr
LoadingAnimation::Create(vrb::CreationContextPtr aContext) {
  return std::make_shared<vrb::ConcreteClass<LoadingAnimation, LoadingAnimation::State> >(aContext);
}


LoadingAnimation::LoadingAnimation(State& aState, vrb::CreationContextPtr& aContext) : m(aState) {
  m.context = aContext;
  m.Initialize();
}

} // namespace crow

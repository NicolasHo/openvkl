// ======================================================================== //
// Copyright 2019 Intel Corporation                                         //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "Renderer.h"

namespace openvkl {
  namespace examples {

    struct RayMarchIterator : public Renderer
    {
      RayMarchIterator(VKLVolume volume);
      ~RayMarchIterator() override = default;

      void commit() override;

      void renderFrame() override;

      Ray computeRay(const vec2f &screenCoords) const override;
      Ray computeRay(const vec2f &screenCoords, const float &c) const;
      
      vec3f renderPixel(Ray &ray, const vec4i &sampleID) override;

     private:
      float samplingRate{1.f};
      Ray camera;
      int nbrays=64;
    };
      
    inline Ray RayMarchIterator::computeRay(const vec2f &screenCoords, const float &c) const
    {
      vec3f dir = dir_00 + screenCoords.x * dir_du + screenCoords.y * dir_dv;

      Ray ray;

      ray.org = camera.org;
      ray.dir = normalize(dir) * c;
      ray.t   = range1f(0.f, ospcommon::inf);

      return ray;
    };

    inline Ray RayMarchIterator::computeRay(const vec2f &screenCoords) const
    {
      vec3f org = camPos;
      vec3f dir = dir_00 + screenCoords.x * dir_du + screenCoords.y * dir_dv;

      Ray ray;

      ray.org = org;
      ray.dir = normalize(dir);
      ray.t   = range1f(0.f, ospcommon::inf);

      return ray;
    };

  }  // namespace examples
}  // namespace openvkl
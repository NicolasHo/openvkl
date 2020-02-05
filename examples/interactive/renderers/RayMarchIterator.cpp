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

#include "RayMarchIterator.h"
#include "ospcommon/tasking/parallel_for.h"
// ispc
#include "RayMarchIterator_ispc.h"

namespace openvkl {
  namespace examples {

    RayMarchIterator::RayMarchIterator(VKLVolume volume) : Renderer(volume)
    {
      ispcEquivalent = ispc::RayMarchIterator_create(volume);
    }

    void RayMarchIterator::commit()
    {
      Renderer::commit();

      samplingRate = getParam<float>("samplingRate", 1.f);

      ispc::RayMarchIterator_set(ispcEquivalent, samplingRate);
    }


    void RayMarchIterator::renderFrame()
    {
      auto fbDims = pixelIndices.dimensions();

      vec2f screen(fbDims.x/2 * rcp(float(fbDims.x)),
                   fbDims.y/2 * rcp(float(fbDims.y)));

      Ray initRay = computeRay(screen);

      initRay.t = intersectRayBox(initRay.org, initRay.dir, volumeBounds);

      if (initRay.t.empty())
        return;      
        
      initRay.org = initRay.org + initRay.dir*initRay.t.lower;
      float coef = (initRay.t.upper-initRay.t.lower) / fbDims.y;
      initRay.dir = initRay.dir * coef;
      initRay.org = initRay.org - initRay.dir;

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      for (int i = 0; i < spp; ++i) {
        float accumScale = 1.f / (frameID + 1);

        tasking::parallel_for((size_t)fbDims.x, [&](size_t i) {

          auto pixel = pixelIndices.reshape(i + (fbDims.y/2)*fbDims.x);

          vec2f screen(pixel.x * rcp(float(fbDims.x)),
                       pixel.y * rcp(float(fbDims.y)));

          Ray ray = computeRay(screen, initRay.org, coef);

          tasking::parallel_for((size_t)fbDims.y, [&](size_t j) 
          {
            size_t id = i + j*fbDims.x;

            framebuffer[id] = vec3f(1.f, 0.f, 0.f);// * accumScale;

            // linear to sRGB color space conversion
            framebuffer[id] = vec3f(pow(framebuffer[id].x, 1.f / 2.2f),
                                 pow(framebuffer[id].y, 1.f / 2.2f),
                                 pow(framebuffer[id].z, 1.f / 2.2f));
          });

          vec3f color = renderPixel(ray, vec4i(pixel.x, pixel.y, fbDims.y, fbDims.x));




        });

        frameID++;
      }
    }

#ifndef RAYMARCHER_ITERATOR_TESTS
    vec3f RayMarchIterator::renderPixel(Ray &ray, const vec4i &sampleID)
    {
      ray.t = intersectRayBox(ray.org, ray.dir, volumeBounds);
      // ray.org = ray.org + ray.dir*ray.t.lower;

      if (ray.t.empty())
        return vec3f(0.f);

      // ray.dir = (ray.dir * (ray.t.upper-ray.t.lower)) / sampleID[2];

      tasking::parallel_for((size_t)sampleID[2], [&](size_t j) 
      {
        size_t id = sampleID[0] + j*(size_t)sampleID[3];
        float sample;

        const vec3f c = ray.org + j* ray.dir;
        sample        = vklComputeSample(volume, (const vkl_vec3f *)&c);
        // vec4f sampleColorAndOpacity = sampleTransferFunction(sample);
        vec3f pixel_color = sample; //vec3f(sampleColorAndOpacity) * sampleColorAndOpacity.w;

        framebuffer[id] = vec3f(pixel_color.x, pixel_color.y, pixel_color.z);

        // linear to sRGB color space conversion
        framebuffer[id] = vec3f(pow(framebuffer[id].x, 1.f / 2.2f),
                              pow(framebuffer[id].y, 1.f / 2.2f),
                              pow(framebuffer[id].z, 1.f / 2.2f));
      });

      return vec3f(0.f);
    }

#else
    vec3f RayMarchIterator::renderPixel(Ray &ray, const vec4i &sampleID)
    { 
      int inter_loop = 1;
      size_t id = sampleID[0];

      vec3f color(0.f);
      float alpha = 0.f;

      // create volume iterator
      vkl_range1f tRange;
      tRange.lower = ray.t.lower;
      tRange.upper = ray.t.upper;

      VKLIntervalIterator iterator;
      vklInitIntervalIterator(&iterator,
                              volume,
                              (vkl_vec3f *)&ray.org,
                              (vkl_vec3f *)&ray.dir,
                              &tRange,
                              valueSelector);

      // the current ray interval
      VKLInterval interval;

      while (vklIterateInterval(&iterator, &interval) && inter_loop < sampleID[2]) {
        const float nominalSamplingDt = interval.nominalDeltaT / samplingRate;

        // initial sub interval, based on our renderer-defined sampling rate
        // and the volume's nominal dt
        box1f subInterval(interval.tRange.lower,
                          min(interval.tRange.lower + nominalSamplingDt,
                              interval.tRange.upper));

        // integrate as long as we have valid sub intervals and are not
        // fully opaque
        while (subInterval.upper - subInterval.lower > 0.f && inter_loop < sampleID[2]) {
          const float t  = 0.5f * (subInterval.lower + subInterval.upper);
          const float dt = subInterval.upper - subInterval.lower;

          // get volume sample
          vec3f c      = ray.org + t * ray.dir;
          float sample = vklComputeSample(volume, (vkl_vec3f *)&c);

          // map through transfer function
          vec4f sampleColorAndOpacity = sampleTransferFunction(sample);

          // accumulate contribution
          const float clampedOpacity = clamp(sampleColorAndOpacity.w * dt);

          sampleColorAndOpacity = sampleColorAndOpacity * clampedOpacity;

          color = /*color + (1.f - alpha) */ vec3f(sampleColorAndOpacity);
          //alpha = alpha + (1.f - alpha) * clampedOpacity;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////
          framebuffer[id] = vec3f(color.x, color.y, color.z);

          // linear to sRGB color space conversion
          framebuffer[id] = vec3f(pow(framebuffer[id].x, 1.f / 2.2f),
                                pow(framebuffer[id].y, 1.f / 2.2f),
                                pow(framebuffer[id].z, 1.f / 2.2f));

          id += (size_t)sampleID[3];
          inter_loop += 1;
  ///////////////////////////////////////////////////////////////////////////////////////////////////////

          // compute next sub interval
          subInterval.lower = subInterval.upper;
          subInterval.upper =
              min(subInterval.lower + nominalSamplingDt, interval.tRange.upper);
        }
      }

      return color;
    }
#endif

  }  // namespace examples
}  // namespace openvkl

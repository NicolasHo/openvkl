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
      nbrays       = getParam<int>("nbrays", 64);

      ispc::RayMarchIterator_set(ispcEquivalent, samplingRate);
    }


    void RayMarchIterator::renderFrame()
    {
      auto fbDims = pixelIndices.dimensions();

      vec2f screen(fbDims.x/2 * rcp(float(fbDims.x)),
                   fbDims.y/2 * rcp(float(fbDims.y)));

      camera = computeRay(screen);

      camera.t = intersectRayBox(camera.org, camera.dir, volumeBounds);

      if (camera.t.empty())
        return;      
        
      camera.org = camera.org + camera.dir*camera.t.lower;
      float coef = (camera.t.upper-camera.t.lower) / fbDims.y;
      camera.dir = camera.dir * coef;
      camera.org = camera.org - camera.dir;

      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      for (int i = 0; i < spp; ++i) {
        float accumScale = 1.f / (frameID + 1);

        // std::cout << nbrays << std::endl;

        tasking::parallel_for((size_t)fbDims.x, [&](size_t i) {

          auto pixel = pixelIndices.reshape(i + (fbDims.y/2)*fbDims.x);

          vec2f screen(pixel.x * rcp(float(fbDims.x)),
                       pixel.y * rcp(float(fbDims.y)));

          Ray ray = computeRay(screen, coef);

          tasking::parallel_for((size_t)fbDims.y, [&](size_t j) 
          {
            size_t id = i + j*fbDims.x;

            framebuffer[id] = vec3f(0.f, 0.f, 0.f);// * accumScale;

            // linear to sRGB color space conversion
            framebuffer[id] = vec3f(pow(framebuffer[id].x, 1.f / 2.2f),
                                 pow(framebuffer[id].y, 1.f / 2.2f),
                                 pow(framebuffer[id].z, 1.f / 2.2f));
          });

          // vec3f color = renderPixel(ray, vec4i(pixel.x, pixel.y, fbDims.y, fbDims.x));
          vec3f color = renderPixel(ray, vec4i(i, pixel.y, fbDims.x, fbDims.y));




        });

        frameID++;
      }
    }


#ifndef RAYMARCHER_ITERATOR_TESTS
    vec3f RayMarchIterator::renderPixel(Ray &ray, const vec4i &sampleID)
    {
      float r = (sampleID[0]%10 < 5 )?255:0;
      float g = ((sampleID[0]+5)%10 < 5 && sampleID[0] < sampleID[2]/2)?255:0;
      float b = ((sampleID[0]+5)%10 < 5 && sampleID[0] > sampleID[2]/2)?255:0;

      if(sampleID[0] == sampleID[2]/2)
      {
        r = 255;
        g = 255;
        b = 255;
      }

      ray.t = intersectRayBox(ray.org, ray.dir, volumeBounds);
      // ray.org = ray.org + ray.dir*ray.t.lower;

      if (ray.t.empty())
        return vec3f(0.f);

      // ray.dir = (ray.dir * (ray.t.upper-ray.t.lower)) / sampleID[2];

      float angle = std::abs((((float)sampleID[0] / (float)sampleID[2] -0.5f) * fov) / 180.0 * M_PI);
      // std::cout << sampleID[0] << " " << sin(angle) << std::endl;

      int y = static_cast<int>(cos(angle)*(float)sampleID[3]-1);
      int x = static_cast<int>(sin(angle)*(float)sampleID[3]-1);

      int id = (sampleID[2]/2 + x) + y*sampleID[2];
      // vec2i tmp = pixelIndices.reshape(id);
      // std::cout << sampleID[0] << " x:" << x << " y:" << y << std::endl;
      // std::cout << sampleID[0] << " x:" << tmp.x << " y:" << tmp.y << std::endl;

      tasking::parallel_for(sampleID[3], [&](int j) 
      {
        int y = static_cast<int>(cos(angle)*(float)j);
        int x = static_cast<int>(sin(angle)*(float)j);

        if(x < sampleID[2]/2 && x > -sampleID[2]/2)
        {
          int id = x + y*sampleID[2];

          float sample;

          const vec3f c = ray.org + j* ray.dir;
          sample        = vklComputeSample(volume, (const vkl_vec3f *)&c);
          vec3f pixel_color = sample;

          // framebuffer[id] = vec3f(pixel_color.x, pixel_color.y, pixel_color.z);
          // framebuffer[id] = vec3f(r, g + framebuffer[id].y, b + framebuffer[id].z);
          framebuffer[id] = vec3f(r, g, b);

          // linear to sRGB color space conversion
          // framebuffer[id] = vec3f(pow(framebuffer[id].x, 1.f / 2.2f),
          //                       pow(framebuffer[id].y, 1.f / 2.2f),
          //                       pow(framebuffer[id].z, 1.f / 2.2f));
        }
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

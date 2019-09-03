// ======================================================================== //
// Copyright 2018 Intel Corporation                                         //
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

#include "../common/ManagedObject.h"
#include "../common/objectFactory.h"
#include "../iterator/Iterator.h"
#include "openvkl/openvkl.h"
#include "ospcommon/math/box.h"

#define THROW_NOT_IMPLEMENTED                          \
  throw std::runtime_error(std::string(__FUNCTION__) + \
                           " not implemented in this volume!")

using namespace ospcommon;

namespace openvkl {
  namespace ispc_driver {

    template <int W>
    struct Volume : public ManagedObject
    {
      Volume()                   = default;
      virtual ~Volume() override = default;

      static Volume *createInstance(const std::string &type);

      // volumes must provide their own iterator implementations based on
      // their internal acceleration structures.

      // initialize a new iterator for the given input rays (specified by
      // origin, direction and tRange) and optional samplesMask indicating
      // volume sample values of interest. if no samplesMask is provided, all
      // intervals intersecting the volume should be (iteratively) returned by
      // iterateIntervalV(), and no hits should be returned by iterateHitV().
      //
      // the iterator objects can be casted to volume-specific iterator types,
      // and may maintain internal state as desired, e.g. for current state
      // within an acceleration structure, etc.

      virtual void initIntervalIteratorV(const vintn<W> &valid,
                                         vVKLIntervalIteratorN<W> &iterator,
                                         const vvec3fn<W> &origin,
                                         const vvec3fn<W> &direction,
                                         const vrange1fn<W> &tRange,
                                         const SamplesMask *samplesMask);

      // for each active lane / ray (indicated by valid), iterate once for the
      // given iterator and return the next interval (if any) satisfying the
      // iterator's samplesMask in interval. result (0 or 1) should
      // indicate if a new interval was found for each active lane.
      //
      // iterator may be modified to track any internal state as desired.
      virtual void iterateIntervalV(const int *valid,
                                    vVKLIntervalIteratorN<W> &iterator,
                                    vVKLIntervalN<W> &interval,
                                    vintn<W> &result);

      virtual void initHitIteratorV(const vintn<W> &valid,
                                    vVKLHitIteratorN<W> &iterator,
                                    const vvec3fn<W> &origin,
                                    const vvec3fn<W> &direction,
                                    const vrange1fn<W> &tRange,
                                    const SamplesMask *samplesMask);

      // for each active lane / ray (indicated by valid), iterate once for the
      // given iterator and return the next hit (if any) satisfying
      // the iterator's samplesMask in hit. result (0 or 1) should
      // indicate if a new hit was found for each active lane.
      //
      // iterator may be modified to track any internal state as desired.
      virtual void iterateHitV(const int *valid,
                               vVKLHitIteratorN<W> &iterator,
                               vVKLHitN<W> &hit,
                               vintn<W> &result);

      virtual SamplesMask *newSamplesMask();

      virtual void computeSampleV(const int *valid,
                                  const vvec3fn<W> &objectCoordinates,
                                  vfloatn<W> &samples) const = 0;

      virtual vec3f computeGradient(const vec3f &objectCoordinates) const = 0;
      virtual box3f getBoundingBox() const                                = 0;
    };

    // Inlined definitions ////////////////////////////////////////////////////

    template <int W>
    inline Volume<W> *Volume<W>::createInstance(const std::string &type)
    {
      return createInstanceHelper<Volume<W>, VKL_VOLUME>(type);
    }

    template <int W>
    inline void Volume<W>::initIntervalIteratorV(
        const vintn<W> &valid,
        vVKLIntervalIteratorN<W> &iterator,
        const vvec3fn<W> &origin,
        const vvec3fn<W> &direction,
        const vrange1fn<W> &tRange,
        const SamplesMask *samplesMask)
    {
      THROW_NOT_IMPLEMENTED;
    }

    template <int W>
    inline void Volume<W>::iterateIntervalV(const int *valid,
                                            vVKLIntervalIteratorN<W> &iterator,
                                            vVKLIntervalN<W> &interval,
                                            vintn<W> &result)
    {
      THROW_NOT_IMPLEMENTED;
    }

    template <int W>
    inline void Volume<W>::initHitIteratorV(const vintn<W> &valid,
                                            vVKLHitIteratorN<W> &iterator,
                                            const vvec3fn<W> &origin,
                                            const vvec3fn<W> &direction,
                                            const vrange1fn<W> &tRange,
                                            const SamplesMask *samplesMask)
    {
      THROW_NOT_IMPLEMENTED;
    }

    template <int W>
    inline void Volume<W>::iterateHitV(const int *valid,
                                       vVKLHitIteratorN<W> &iterator,
                                       vVKLHitN<W> &hit,
                                       vintn<W> &result)
    {
      THROW_NOT_IMPLEMENTED;
    }

    template <int W>
    inline SamplesMask *Volume<W>::newSamplesMask()
    {
      THROW_NOT_IMPLEMENTED;
    }

#define VKL_REGISTER_VOLUME(InternalClass, external_name) \
  VKL_REGISTER_OBJECT(                                    \
      ::openvkl::ManagedObject, volume, InternalClass, external_name)

  }  // namespace ispc_driver
}  // namespace openvkl

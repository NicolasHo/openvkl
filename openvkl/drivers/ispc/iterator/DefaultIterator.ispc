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

#include "../math/box_utility.ih"
#include "../value_selector/ValueSelector.ih"
#include "../volume/Volume.ih"
#include "DefaultIterator.ih"

export uniform int DefaultIterator_sizeOf()
{
  return sizeof(varying DefaultIterator);
}

export void DefaultIterator_Initialize(const int *uniform imask,
                                       void *uniform _self,
                                       void *uniform _volume,
                                       void *uniform _origin,
                                       void *uniform _direction,
                                       void *uniform _tRange,
                                       void *uniform _valueSelector,
                                       const uniform box3f &boundingBox,
                                       const uniform box1f &valueRange)
{
  if (!imask[programIndex]) {
    return;
  }

  varying DefaultIterator *uniform self =
      (varying DefaultIterator * uniform) _self;

  self->volume        = (Volume * uniform) _volume;
  self->origin        = *((varying vec3f * uniform) _origin);
  self->direction     = *((varying vec3f * uniform) _direction);
  self->tRange        = *((varying box1f * uniform) _tRange);
  self->valueSelector = (uniform ValueSelector * uniform) _valueSelector;
  self->valueRange    = valueRange;

  self->boundingBoxTRange =
      intersectBox(self->origin, self->direction, boundingBox, self->tRange);

  // compute a nominal interval length as a fraction of the largest bounding box
  // dimension
  uniform float bbMaxDimension =
      reduce_max(boundingBox.upper - boundingBox.lower);
  self->nominalIntervalLength = 0.1f * bbMaxDimension;

  resetInterval(self->intervalState.currentInterval);

  self->hitState.tRange = self->boundingBoxTRange;
}

export void *uniform DefaultIterator_getCurrentInterval(void *uniform _self)
{
  varying DefaultIterator *uniform self =
      (varying DefaultIterator * uniform) _self;
  return &self->intervalState.currentInterval;
}

export void DefaultIterator_iterateInterval(const int *uniform imask,
                                            void *uniform _self,
                                            uniform int *uniform _result)
{
  if (!imask[programIndex]) {
    return;
  }

  varying DefaultIterator *uniform self =
      (varying DefaultIterator * uniform) _self;

  varying int *uniform result = (varying int *uniform)_result;

  if (isempty1f(self->boundingBoxTRange)) {
    *result = false;
    return;
  }

  if (self->valueSelector &&
      !overlaps1f(self->valueSelector->rangesMinMax, self->valueRange)) {
    *result = false;
    return;
  }

  Interval nextInterval;

  nextInterval.tRange.lower =
      max(self->intervalState.currentInterval.tRange.upper,
          self->boundingBoxTRange.lower);
  nextInterval.tRange.upper =
      min(nextInterval.tRange.lower + self->nominalIntervalLength,
          self->boundingBoxTRange.upper);

  if (nextInterval.tRange.upper <= nextInterval.tRange.lower) {
    *result = false;
    return;
  }

  // conservatively use the volume value range
  nextInterval.valueRange = self->valueRange;

  nextInterval.nominalDeltaT = 0.25f * self->nominalIntervalLength;

  self->intervalState.currentInterval = nextInterval;
  *result                             = true;
}

export void *uniform DefaultIterator_getCurrentHit(void *uniform _self)
{
  varying DefaultIterator *uniform self =
      (varying DefaultIterator * uniform) _self;
  return &self->hitState.currentHit;
}

export void DefaultIterator_iterateHit(const int *uniform imask,
                                       void *uniform _self,
                                       uniform int *uniform _result)
{
  if (!imask[programIndex]) {
    return;
  }

  varying DefaultIterator *uniform self =
      (varying DefaultIterator * uniform) _self;

  varying int *uniform result = (varying int *uniform)_result;

  if (isempty1f(self->hitState.tRange)) {
    *result = false;
    return;
  }

  cif(!self->valueSelector || self->valueSelector->numValues == 0)
  {
    *result = false;
    return;
  }

  // this is an estimated step that may yield artifacts
  const uniform float step = 0.1f * self->nominalIntervalLength;

  float surfaceEpsilon;

  bool foundHit = intersectSurfaces(self->volume,
                                    self->origin,
                                    self->direction,
                                    self->hitState.tRange,
                                    step,
                                    self->valueSelector->numValues,
                                    self->valueSelector->values,
                                    self->hitState.currentHit,
                                    surfaceEpsilon);

  if (foundHit) {
    *result                     = true;
    self->hitState.tRange.lower = self->hitState.currentHit.t + surfaceEpsilon;
  } else {
    *result                     = false;
    self->hitState.tRange.lower = inf;
  }
}

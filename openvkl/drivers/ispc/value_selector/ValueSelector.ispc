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

#include "ValueSelector.ih"

export void *uniform ValueSelector_Constructor(void *uniform volume,
                                               const uniform int &numRanges,
                                               const box1f *uniform ranges,
                                               const uniform int &numValues,
                                               const float *uniform values)
{
  uniform ValueSelector *uniform self = uniform new uniform ValueSelector;

  self->volume = volume;

  self->numRanges = numRanges;
  self->ranges    = uniform new uniform box1f[numRanges];

  foreach (i = 0 ... numRanges) {
    self->ranges[i] = ranges[i];
  }

  self->rangesMinMax = make_box1f(inf, -inf);

  foreach (i = 0... numRanges) {
    self->rangesMinMax.lower =
        min(self->rangesMinMax.lower, reduce_min(ranges[i].lower));
    self->rangesMinMax.upper =
        max(self->rangesMinMax.upper, reduce_max(ranges[i].upper));
  }

  self->numValues = numValues;
  self->values    = uniform new uniform float[numValues];

  foreach (i = 0 ... numValues) {
    self->values[i] = values[i];
  }

  self->valuesMinMax = make_box1f(inf, -inf);

  foreach (i = 0... numValues) {
    self->valuesMinMax.lower =
        min(self->valuesMinMax.lower, reduce_min(values[i]));
    self->valuesMinMax.upper =
        max(self->valuesMinMax.upper, reduce_max(values[i]));
  }

  return self;
}

export void *uniform ValueSelector_Destructor(void *uniform _self)
{
  uniform ValueSelector *uniform self = (uniform ValueSelector * uniform) _self;
  delete[] self->ranges;
  delete[] self->values;
  delete self;
}

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

#include "TestingStructuredVolume.h"
// std
#include <algorithm>
#include <fstream>


#include "./dcm/data_element.h"
#include "./dcm/data_sequence.h"
#include "./dcm/data_set.h"
#include "./dcm/dicom_file.h"
#include "./dcm/logger.h"
#include "./dcm/visitor.h"

#include <dirent.h> 
#include <algorithm> 
#include <experimental/filesystem>


using namespace ospcommon;

namespace openvkl {
  namespace testing {

    struct RawFileStructuredVolume : public TestingStructuredVolume
    {
      RawFileStructuredVolume(const std::string &filename,
                              std::string &gridType,
                              vec3i &dimensions,
                              vec3f &gridOrigin,
                              vec3f &gridSpacing,
                              VKLDataType voxelType);

      void initDicomFormat( std::string &gridType,
                            vec3i &dimensions,
                            vec3f &gridOrigin,
                            vec3f &gridSpacing,
                            VKLDataType voxelType);

      std::vector<unsigned char> generateVoxels() override;
      std::vector<unsigned char> generateVoxelsDicom();

     private:
      std::string filename;
    };

    // Inlined definitions ////////////////////////////////////////////////////

    inline RawFileStructuredVolume::RawFileStructuredVolume(
        const std::string &filename,
        std::string &gridType,
        vec3i &dimensions,
        vec3f &gridOrigin,
        vec3f &gridSpacing,
        VKLDataType voxelType)
        : filename(filename),
          TestingStructuredVolume(
              gridType, dimensions, gridOrigin, gridSpacing, voxelType)
    {      
      if(std::experimental::filesystem::is_directory(std::experimental::filesystem::status(filename)))
        initDicomFormat(gridType, dimensions, gridOrigin, gridSpacing, voxelType);
    }


    inline void RawFileStructuredVolume::initDicomFormat(
        std::string &_gridType,
        vec3i &_dimensions,
        vec3f &_gridOrigin,
        vec3f &_gridSpacing,
        VKLDataType _voxelType)
    {
      // filename = directory path

      std::vector<std::string> files; 

      DIR           *dirp;
      struct dirent *directory;

      dirp = opendir(filename.c_str());
      if (dirp)
      {
          while ((directory = readdir(dirp)) != NULL)
          {
            std::string file = filename + directory->d_name;
            if(file.substr(file.find_last_of(".") + 1) == "dcm") 
              files.push_back(file);
          }

          closedir(dirp);
      }

      if(files.size() == 0)
        throw std::runtime_error("error no dicom file found");

      // loading of a random slice
      dcm::DicomFile dicom_file(files[0]);

      if (!dicom_file.Load()) {
        throw std::runtime_error("error opening dicom file");
      }

      // setting the TestingStructuredVolume attributes
      std::uint16_t rows;
      std::uint16_t columns;
      if (!dicom_file.GetUint16(dcm::tags::kRows, &rows))
        throw std::runtime_error("error reading rows volume file");
      if (!dicom_file.GetUint16(dcm::tags::kColumns, &columns))
        throw std::runtime_error("error reading columns volume file");
      dimensions  = vec3f(rows,columns,files.size());

      // Pixel Aspect Ratio (0028,0034)
      // Pixel Padding Range Limit (0028,0121)
      // Slice Thickness (0018,0050)
      std::string ratio;
      std::string thickness;
      if (!dicom_file.GetString(dcm::tags::kPixelSpacing, &ratio))
        throw std::runtime_error("error reading columns volume file");
      if (!dicom_file.GetString(0x00180050, &thickness))
        throw std::runtime_error("error reading columns volume file");
      std::cout << "ratio=" << ratio << " thickness="<< thickness << std::endl; 
      gridSpacing = vec3f(1,1,1);

      // Slice Location ?
      gridOrigin = vec3f(0,0,0);

      // TO-DO
      gridType    = "structured_regular";

      // Pixel Representation (0028,0103)
      voxelType   = VKL_FLOAT;

      _gridType     = gridType;
      _dimensions   = dimensions;
      _gridOrigin   = gridOrigin;
      _gridSpacing  = gridSpacing;
      _voxelType    = voxelType;
    }

    inline std::vector<unsigned char> RawFileStructuredVolume::generateVoxelsDicom()
    {
      
      // generation of the files list
      std::vector<std::string> files; 

      DIR           *dirp;
      struct dirent *directory;

      dirp = opendir(filename.c_str());
      if (dirp)
      {
          while ((directory = readdir(dirp)) != NULL)
          {
            std::string file = filename + directory->d_name;
            if(file.substr(file.find_last_of(".") + 1) == "dcm") 
              files.push_back(file);
          }

          closedir(dirp);
      }
      // sorting slices
      std::sort(files.begin(), files.end()); 

      // creation of the voxels list
      auto numValues = this->dimensions.long_product();
      std::vector<unsigned char> voxels(numValues *
                                        sizeOfVKLDataType(voxelType));

      // loading voxels
      int imageSize = this->dimensions.x*this->dimensions.y;
      uint i, k = 0;

      for (i=0; i<files.size(); i++)
      {  
        dcm::DicomFile dicom_file(files[i]);

        if (!dicom_file.Load()) {
          throw std::runtime_error("error opening raw volume file");
        }

        const dcm::DataElement* element = dicom_file.Get(dcm::tags::kPixelData);
        k = 0;
        for (std::vector<char>::const_iterator j = element->buffer().begin(); j != element->buffer().end(); ++(++j))
        {
          voxels[i*imageSize+k] = (unsigned char)i;
          k += 1;
        }
        // throw std::runtime_error("error reading raw volume file");
        //std::cout << (i-1)*imageSize+k << std::endl;
      }
      
      std::cout << (i-1)*imageSize+k << "/" << this->dimensions.long_product() << " voxels" << std::endl;
        
      return voxels;
    
    }

    inline std::vector<unsigned char> RawFileStructuredVolume::generateVoxels()
    {
      if(std::experimental::filesystem::is_directory(std::experimental::filesystem::status(filename)))
      {
        return generateVoxelsDicom();
      }

      auto numValues = this->dimensions.long_product();
      std::vector<unsigned char> voxels(numValues *
                                        sizeOfVKLDataType(voxelType));

      std::ifstream input(filename, std::ios::binary);

      if (!input) {
        throw std::runtime_error("error opening raw volume file");
      }

      input.read(
          (char *)voxels.data(),
          this->dimensions.long_product() * sizeOfVKLDataType(voxelType));

      if (!input.good()) {
        throw std::runtime_error("error reading raw volume file");
      }

      return voxels;
    }

  }  // namespace testing
}  // namespace openvkl

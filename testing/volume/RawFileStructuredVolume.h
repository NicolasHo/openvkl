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

#include <CImg.h>

#include <imebra/imebra.h>

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

      int width;
      int height;

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
      {
        dirp = opendir((filename + "/CTScanData/").c_str());
        if (dirp)
        {
            while ((directory = readdir(dirp)) != NULL)
            {
              std::string file = filename + "/CTScanData/" + directory->d_name;
              if(file.substr(file.find_last_of(".") + 1) == "jpg") 
                files.push_back(file);
            }

            closedir(dirp);
        }
        if(files.size() == 0)
          throw std::runtime_error("error no dicom files (or images) found");

        cimg_library::CImg<unsigned char> image(files[0].c_str());
        width = image.width();
        height = image.height();
        std::cout << "JPG | width=" << width << " height="<< height << std::endl; 
      }
      else
      {
        imebra::DataSet loadedDataSet(imebra::CodecFactory::load(files[1], 2048));
        // Retrieve the first image (index = 0)
        imebra::Image image(loadedDataSet.getImageApplyModalityTransform(0));

        // Get the size in pixels
        width = image.getWidth();
        height = image.getHeight();
        std::cout << "Dicom | width=" << width << " height="<< height << std::endl; 
      }
      


      /////////////////////////////////////////////////////////////////////////////////////////////

      // imebra::DataSet loadedDataSet(imebra::CodecFactory::load(files[1], 2048));
      // // Retrieve the first image (index = 0)
      // imebra::Image image(loadedDataSet.getImageApplyModalityTransform(0));

      // // Get the color space
      // std::string colorSpace = image.getColorSpace();

      // // Get the size in pixels
      // std::uint32_t width = image.getWidth();
      // std::uint32_t height = image.getHeight();
      // std::cout << colorSpace << " | width=" << width << " height="<< height << std::endl; 

      /////////////////////////////////////////////////////////////////////////////////////////////

      // loading of a random slice
      // dcm::DicomFile dicom_file(files[0]);

      // if (!dicom_file.Load()) {
      //   throw std::runtime_error("error opening dicom file");
      // }

      // std::string tmp;
      // if (dicom_file.GetString(0x00200013, &tmp))
      //   std::cout << "InstanceNumber:" << tmp << std::endl;
      // if (dicom_file.GetString(0x00200012, &tmp))
      //   std::cout << "Acquisition Number:" << tmp << std::endl;
      // if (dicom_file.GetString(0x00201002, &tmp))
      //   std::cout << "InstanceNumber:" << tmp << std::endl; 
      
      // setting the TestingStructuredVolume attributes
      // std::uint16_t rows;
      // std::uint16_t columns;
      // if (!dicom_file.GetUint16(dcm::tags::kRows, &rows))
      //   throw std::runtime_error("error reading rows volume file");
      // if (!dicom_file.GetUint16(dcm::tags::kColumns, &columns))
      //   throw std::runtime_error("error reading columns volume file");
      dimensions  = vec3i(width,height,files.size());

      // Pixel Aspect Ratio (0028,0034)
      // Pixel Padding Range Limit (0028,0121)
      // Slice Thickness (0018,0050)
      // std::string ratio;
      // std::string thickness;
      // if (!dicom_file.GetString(dcm::tags::kPixelSpacing, &ratio))
      //   throw std::runtime_error("error reading columns volume file");
      // if (!dicom_file.GetString(0x00180050, &thickness))
      //   throw std::runtime_error("error reading columns volume file");
      // std::cout << "ratio=" << ratio << " thickness="<< thickness << std::endl; 
      gridSpacing = vec3f(1,1,1);

      // Slice Location ?
      gridOrigin = vec3f(0,0,0);

      // TO-DO
      gridType    = "structured_regular";

      // Pixel Representation (0028,0103)
      voxelType   = VKL_USHORT;

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
      std::vector<std::string> segm; 

      DIR           *dirp;
      struct dirent *directory;
      bool areImages = false;

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
      {
        dirp = opendir((filename + "/CTScanData").c_str());
        if (dirp)
        {
            while ((directory = readdir(dirp)) != NULL)
            {
              std::string file = filename + "/CTScanData/" + directory->d_name;
              if(file.substr(file.find_last_of(".") + 1) == "jpg") 
                files.push_back(file);
                
              std::string file_seg = filename + "/LabelledData/" + directory->d_name;
              if(file_seg.substr(file_seg.find_last_of(".") + 1) == "jpg") 
                segm.push_back(file_seg);
                
            }

            closedir(dirp);
        }
        areImages = true;
      }

      // sorting slices
      std::sort(files.begin(), files.end());
      if(segm.size() != 0) 
        std::sort(segm.begin(), segm.end()); 

      // creation of the voxels list
      auto numValues = this->dimensions.long_product();


      int voxel_size = sizeOfVKLDataType(voxelType) + sizeof(uint8_t);
      std::vector<unsigned char> voxels(numValues *voxel_size);

      // loading voxels
      int imageSize = this->dimensions.x*this->dimensions.y;

      std::uint32_t width = this->dimensions.x;
      std::uint32_t height = this->dimensions.y;

      uint i, k = voxel_size * height * width;
      int imageNumber;

      for (i=0; i<files.size(); i++)
      {  

        // dcm::DicomFile dicom_file(files[i]);

        // if (!dicom_file.Load()) {
        //   throw std::runtime_error("error opening raw volume file");
        // }

        // const dcm::DataElement* element = dicom_file.Get(dcm::tags::kPixelData);
        // k = 0;
        // for (std::vector<char>::const_iterator j = element->buffer().begin(); j != element->buffer().end(); ++(++j))
        // {
        //   voxels[i*imageSize+k] = (unsigned char)i;
        //   k += 1;
        // }



        // imebra::DataSet loadedDataSet(imebra::CodecFactory::load(files[i], 2048));
        // // Retrieve the first image (index = 0)
        // imebra::Image image(loadedDataSet.getImageApplyModalityTransform(0));

        // // Retrieve the data handler
        // imebra::ReadingDataHandlerNumeric dataHandler(image.getReadingDataHandler());

        // for(std::uint32_t scanY(0); scanY != height; ++scanY)
        // {
        //   for(std::uint32_t scanX(0); scanX != width; ++scanX)
        //   {
        //     // For monochrome images
        //     std::int32_t luminance = dataHandler.getSignedLong(scanY * width + scanX);
        //     //voxels[4*(scanY * width + scanX) + k] = luminance;
        //     voxels[4*(scanY * width + scanX) + k] = (luminance >> 24) & 0xFF;
        //     voxels[4*(scanY * width + scanX) + k +1] = (luminance >> 16) & 0xFF;
        //     voxels[4*(scanY * width + scanX) + k +2] = (luminance >> 8) & 0xFF;
        //     voxels[4*(scanY * width + scanX) + k +3] = luminance & 0xFF;
        //   }
        // }
        // k += 4 * height * width;

        if(areImages)
        {
          cimg_library::CImg<unsigned char> image(files[i].c_str());
          cimg_library::CImg<unsigned char> image_seg;
          if(segm.size() != 0) 
            image_seg = cimg_library::CImg<unsigned char>(segm[i].c_str());

          for(int scanY(0); scanY != height; ++scanY)
          {
            for(int scanX(0); scanX != width; ++scanX)
            {
              uint16_t luminance = static_cast<uint16_t>((int)image(scanX,scanY,0,0));
              memcpy(&voxels[voxel_size*(scanY * width + scanX) + (i * k)], &luminance, sizeof(uint16_t));

              if(segm.size() != 0) 
              {
                uint8_t segmentation = static_cast<uint8_t>((int)image_seg(scanX,scanY,0,0));
                memcpy(&voxels[sizeOfVKLDataType(voxelType) + voxel_size*(scanY * width + scanX) + (i * k)], &luminance, sizeof(uint8_t));
              }

            }
          }
        }
        else
        {
          {
            dcm::DicomFile dicom_file(files[i]);

            if (!dicom_file.Load()) {
              throw std::runtime_error("error opening dicom file");
            }

            std::string tmp;
            if (dicom_file.GetString(0x00200013, &tmp))
              imageNumber = std::stoi(tmp);
              // std::cout << "InstanceNumber:" << tmp << std::endl;
          }


          imebra::DataSet loadedDataSet(imebra::CodecFactory::load(files[i], 2048));
          // Retrieve the first image (index = 0)
          imebra::Image image(loadedDataSet.getImageApplyModalityTransform(0));

          // Retrieve the data handler
          imebra::ReadingDataHandlerNumeric dataHandler(image.getReadingDataHandler());

          for(std::uint32_t scanY(0); scanY != height; ++scanY)
          {
            for(std::uint32_t scanX(0); scanX != width; ++scanX)
            {
              // For monochrome images
              float luminance = static_cast<float>(dataHandler.getSignedLong(scanY * width + scanX));
          
              memcpy(&voxels[4*(scanY * width + scanX) + ((imageNumber-1) * k)], &luminance, sizeof(float));

            }
          }
          
        }


        // throw std::runtime_error("error reading raw volume file");
        //std::cout << (i-1)*imageSize+k << std::endl;
      }
      std::cout << (i * k) << "/" << numValues * voxel_size << " bytes loaded" << std::endl;
        
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

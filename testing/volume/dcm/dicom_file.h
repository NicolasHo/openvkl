#ifndef DCM_DICOM_FILE_H_
#define DCM_DICOM_FILE_H_

#include "./data_set.h"

namespace dcm {

class DicomFile : public DataSet {
public:
  explicit DicomFile(const Path& path);

  ~DicomFile() = default;

  // Load DICOM file.
  bool Load();

  // Change transfer syntax.
  bool SetTransferSyntax(const std::string& transfer_syntax_uid);

  // Save DICOM file to the given path.
  bool Save(const Path& new_path);

private:
  Path path_;

  std::string transfer_syntax_uid_;
};

}  // namespace dcm

#endif  // DCM_DICOM_FILE_H_

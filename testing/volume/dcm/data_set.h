#ifndef DCM_DATA_SET_H_
#define DCM_DATA_SET_H_

#include <cstdint>
#include <string>
#include <vector>

#include "./charset.h"
#include "./defs.h"
#include "./data_element.h"
#include "./data_sequence.h"

namespace dcm {

class Visitor;

class DataSet {
public:
  DataSet(VR::Type vr_type = VR::EXPLICIT,
          ByteOrder byte_order = ByteOrder::LE,
          Charset charset = Charset::ISO_IR_6);  // TODO: default charset

  ~DataSet();

  void Accept(Visitor& visitor) const;

  // ---------------------------------------------------------------------------

  VR::Type vr_type() const { return vr_type_; }
  void set_vr_type(VR::Type vr_type) { vr_type_ = vr_type; }

  ByteOrder byte_order() const { return byte_order_; }
  void set_byte_order(ByteOrder byte_order) { byte_order_ = byte_order; }

  Charset charset() const { return charset_; }
  void set_charset(Charset charset) { charset_ = charset; }

  // ---------------------------------------------------------------------------

  std::size_t size() const { return elements_.size(); }

  // Get the element at the given index.
  const DataElement* operator[](std::size_t index) const;

  const DataElement* At(std::size_t index) const {
    return (*this)[index];
  }

  // Get the element with the given tag.
  const DataElement* Get(Tag tag) const;

  // Get the sequence element with the given tag.
  // Return nullptr if the tag doesn't exist or it's not a sequence.
  const DataSequence* GetSequence(Tag tag) const;

  bool Append(DataElement* element);

  bool Insert(DataElement* element);

  void Clear();

  // ---------------------------------------------------------------------------

  // Set VR type for all data sets.
  void SetVRType(VR::Type vr_type);

  // Set byte order for all numeric values.
  void SetByteOrder(ByteOrder byte_order);

#if 0
  // Sum of the length of all elements in the given group except (gggg,0000).
  std::uint32_t GetGroupLength(std::uint16_t group) const;
#endif

  // Update the value of group length element (gggg,0000).
  // This is not effecient since the length will be recalculated by traversing
  // each element in this group recursively.
  // If the group length element (gggg,0000) doesn't exist, just return false
  // without any calculation.
  // TODO: When set a (string) value, update group length automatically.
  bool UpdateGroupLength(std::uint16_t group);

  // ---------------------------------------------------------------------------

  // Get value length.
  // Return false if the tag doesn't exist.
  bool GetVL(Tag tag, std::uint32_t* vl) const;

  // Get value length.
  // Return |vl_default| if the tag doesn't exist.
  std::uint32_t GetVL(Tag tag,
                      std::uint32_t vl_default = kUndefinedLength) const;

  // Get value multiplicity.
  // Return false if the tag doesn't exist.
  bool GetVM(Tag tag, std::size_t* vm) const;

  // Get value multiplicity.
  // Return |vm_default| if the tag doesn't exist.
  std::size_t GetVM(Tag tag, std::size_t vm_default = 0) const;

  // ---------------------------------------------------------------------------
  // String

  bool GetString(Tag tag, std::string* value) const;

  std::string GetString(Tag tag) const;

  bool SetString(Tag tag, const std::string& value);

  bool GetStringArray(Tag tag, std::vector<std::string>* values) const;

  bool SetStringArray(Tag tag, const std::vector<std::string>& values);

  // ---------------------------------------------------------------------------
  // US (Unsigned Short)

  bool GetUint16(Tag tag, std::uint16_t* value) const;

  bool SetUint16(Tag tag, std::uint16_t value);

  bool GetUint16Array(Tag tag, std::vector<std::uint16_t>* values) const;

  bool SetUint16Array(Tag tag, const std::vector<std::uint16_t>& values);

  // ---------------------------------------------------------------------------
  // SS (Signed Short)

  bool GetInt16(Tag tag, std::int16_t* value) const;

  bool SetInt16(Tag tag, std::int16_t value);

  bool GetInt16Array(Tag tag, std::vector<std::int16_t>* values) const;

  bool SetInt16Array(Tag tag, const std::vector<std::int16_t>& values);

  // ---------------------------------------------------------------------------
  // UL (Unsigned Long)

  bool GetUint32(Tag tag, std::uint32_t* value) const;

  bool SetUint32(Tag tag, std::uint32_t value);

  bool GetUint32Array(Tag tag, std::vector<std::uint32_t>* values) const;

  bool SetUint32Array(Tag tag, const std::vector<std::uint32_t>& values);

  // ---------------------------------------------------------------------------
  // SL (Signed Long)

  bool GetInt32(Tag tag, std::int32_t* value) const;

  bool SetInt32(Tag tag, std::int32_t value);

  bool GetInt32Array(Tag tag, std::vector<std::int32_t>* values) const;

  bool SetInt32Array(Tag tag, const std::vector<std::int32_t>& values);

  // ---------------------------------------------------------------------------
  // FL (Floating Point Single)

  bool GetFloat32(Tag tag, float32_t* value) const;

  bool SetFloat32(Tag tag, float32_t value);

  bool GetFloat32Array(Tag tag, std::vector<float32_t>* values) const;

  bool SetFloat32Array(Tag tag, const std::vector<float32_t>& values);

  // ---------------------------------------------------------------------------
  // FD (Floating Point Double)

  bool GetFloat64(Tag tag, float64_t* value) const;

  bool SetFloat64(Tag tag, float64_t value);

  bool GetFloat64Array(Tag tag, std::vector<float64_t>* values) const;

  bool SetFloat64Array(Tag tag, const std::vector<float64_t>& values);

private:
  using Elements = std::vector<DataElement*>;

  // A wrapper of std::lower_bound.
  Elements::iterator LowerBound(Tag tag);

  DataElement* Find(Tag tag);

  // SetXxx template
  bool Set(Tag tag, std::function<bool(DataElement*)> setter);

private:
  // Explicit or implicit VR.
  VR::Type vr_type_;

  // Little endian or big endian.
  ByteOrder byte_order_;

  // Character set.
  Charset charset_;

  Elements elements_;
};

}  // namespace dcm

#endif  // DCM_DATA_SET_H_

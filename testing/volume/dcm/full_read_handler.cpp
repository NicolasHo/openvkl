#include "./full_read_handler.h"

#include <cassert>

#include "./data_sequence.h"
#include "./data_set.h"
#include "./logger.h"

namespace dcm {

FullReadHandler::FullReadHandler(DataSet* data_set)
    : data_set_(data_set) {
}

void FullReadHandler::OnTransferSyntax(VR::Type vr_type, ByteOrder byte_order) {
  data_set_->set_vr_type(vr_type);
  data_set_->set_byte_order(byte_order);
}

bool FullReadHandler::OnElementStart(Tag /*tag*/) {
  return true;  // Go on to read the element.
}

void FullReadHandler::OnElementEnd(DataElement* data_element) {
  // Add the data element to its parent data set.
  AppendElement(data_element);
}

void FullReadHandler::OnSequenceStart(DataSequence* data_sequence) {
  LOG_INFO("OnSequenceStart");

  // Add the data sequence, as a normal element, to its parent data set.
  AppendElement(data_sequence);

  sequence_stack_.push(data_sequence);
}

void FullReadHandler::OnSequenceEnd(DataElement* data_element) {
  if (data_element != nullptr) {
    LOG_INFO("Sequence delimitation tag read.");

    assert(data_element->tag() == tags::kSeqDelimatation);
    assert(!sequence_stack_.empty());

    sequence_stack_.top()->set_delimitation(data_element);

  } else {
    LOG_INFO("Sequence ended.");

    sequence_stack_.pop();
  }
}

void FullReadHandler::OnSequenceItemStart(DataElement* data_element) {
  assert(data_element->tag() == tags::kSeqItemPrefix);
  assert(!sequence_stack_.empty());

  LOG_INFO("OnSequenceItemStart");

  sequence_stack_.top()->NewItem(data_element, data_set_->vr_type(),
                                 data_set_->byte_order());
}

void FullReadHandler::OnSequenceItemEnd(DataElement* data_element) {
  if (data_element != nullptr) {
    LOG_INFO("Sequence item delimitation tag read.");

    assert(data_element->tag() == tags::kSeqItemDelimatation);
    assert(!sequence_stack_.empty());

    sequence_stack_.top()->EndItem(data_element);

  } else {
    LOG_INFO("Sequence item ended.");
  }
}

void FullReadHandler::AppendElement(DataElement* data_element) {
  if (sequence_stack_.empty()) {
    data_set_->Append(data_element);
  } else {
    sequence_stack_.top()->AppendToLastItem(data_element);
  }
}

}  // namespace dcm

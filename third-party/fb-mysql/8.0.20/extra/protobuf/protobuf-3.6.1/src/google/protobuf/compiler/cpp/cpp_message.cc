// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <google/protobuf/compiler/cpp/cpp_message.h>

#include <algorithm>
#include <google/protobuf/stubs/hash.h>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <google/protobuf/compiler/cpp/cpp_enum.h>
#include <google/protobuf/compiler/cpp/cpp_extension.h>
#include <google/protobuf/compiler/cpp/cpp_field.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <google/protobuf/compiler/cpp/cpp_padding_optimizer.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/map_entry_lite.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>



namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

using internal::WireFormat;
using internal::WireFormatLite;

namespace {

template <class T>
void PrintFieldComment(io::Printer* printer, const T* field) {
  // Print the field's (or oneof's) proto-syntax definition as a comment.
  // We don't want to print group bodies so we cut off after the first
  // line.
  DebugStringOptions options;
  options.elide_group_body = true;
  options.elide_oneof_body = true;
  string def = field->DebugStringWithOptions(options);
  printer->Print("// $def$\n",
    "def", def.substr(0, def.find_first_of('\n')));
}

struct FieldOrderingByNumber {
  inline bool operator()(const FieldDescriptor* a,
                         const FieldDescriptor* b) const {
    return a->number() < b->number();
  }
};

// Sort the fields of the given Descriptor by number into a new[]'d array
// and return it.
std::vector<const FieldDescriptor*> SortFieldsByNumber(
    const Descriptor* descriptor) {
  std::vector<const FieldDescriptor*> fields(descriptor->field_count());
  for (int i = 0; i < descriptor->field_count(); i++) {
    fields[i] = descriptor->field(i);
  }
  std::sort(fields.begin(), fields.end(), FieldOrderingByNumber());
  return fields;
}

// Functor for sorting extension ranges by their "start" field number.
struct ExtensionRangeSorter {
  bool operator()(const Descriptor::ExtensionRange* left,
                  const Descriptor::ExtensionRange* right) const {
    return left->start < right->start;
  }
};

bool IsPOD(const FieldDescriptor* field) {
  if (field->is_repeated() || field->is_extension()) return false;
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_ENUM:
    case FieldDescriptor::CPPTYPE_INT32:
    case FieldDescriptor::CPPTYPE_INT64:
    case FieldDescriptor::CPPTYPE_UINT32:
    case FieldDescriptor::CPPTYPE_UINT64:
    case FieldDescriptor::CPPTYPE_FLOAT:
    case FieldDescriptor::CPPTYPE_DOUBLE:
    case FieldDescriptor::CPPTYPE_BOOL:
      return true;
    case FieldDescriptor::CPPTYPE_STRING:
      return false;
    default:
      return false;
  }
}

// Helper for the code that emits the SharedCtor() method.
bool CanConstructByZeroing(const FieldDescriptor* field,
                           const Options& options) {
  bool ret = CanInitializeByZeroing(field);

  // Non-repeated, non-lazy message fields are simply raw pointers, so we can
  // use memset to initialize these in SharedCtor.  We cannot use this in
  // Clear, as we need to potentially delete the existing value.
  ret = ret ||
      (!field->is_repeated() &&
       field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE);
  return ret;
}

// Emits an if-statement with a condition that evaluates to true if |field| is
// considered non-default (will be sent over the wire), for message types
// without true field presence. Should only be called if
// !HasFieldPresence(message_descriptor).
bool EmitFieldNonDefaultCondition(io::Printer* printer,
                                  const string& prefix,
                                  const FieldDescriptor* field) {
  // Merge and serialize semantics: primitive fields are merged/serialized only
  // if non-zero (numeric) or non-empty (string).
  if (!field->is_repeated() && !field->containing_oneof()) {
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
      printer->Print(
          "if ($prefix$$name$().size() > 0) {\n",
          "prefix", prefix,
          "name", FieldName(field));
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      // Message fields still have has_$name$() methods.
      printer->Print(
          "if ($prefix$has_$name$()) {\n",
          "prefix", prefix,
          "name", FieldName(field));
    } else {
      printer->Print(
          "if ($prefix$$name$() != 0) {\n",
          "prefix", prefix,
          "name", FieldName(field));
    }
    printer->Indent();
    return true;
  } else if (field->containing_oneof()) {
    printer->Print(
        "if (has_$name$()) {\n",
        "name", FieldName(field));
    printer->Indent();
    return true;
  }
  return false;
}

// Does the given field have a has_$name$() method?
bool HasHasMethod(const FieldDescriptor* field) {
  if (HasFieldPresence(field->file())) {
    // In proto1/proto2, every field has a has_$name$() method.
    return true;
  }
  // For message types without true field presence, only fields with a message
  // type have a has_$name$() method.
  return field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE;
}

// Collects map entry message type information.
void CollectMapInfo(const Descriptor* descriptor,
                    std::map<string, string>* variables) {
  GOOGLE_CHECK(IsMapEntryMessage(descriptor));
  std::map<string, string>& vars = *variables;
  const FieldDescriptor* key = descriptor->FindFieldByName("key");
  const FieldDescriptor* val = descriptor->FindFieldByName("value");
  vars["key_cpp"] = PrimitiveTypeName(key->cpp_type());
  switch (val->cpp_type()) {
    case FieldDescriptor::CPPTYPE_MESSAGE:
      vars["val_cpp"] = FieldMessageTypeName(val);
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      vars["val_cpp"] = ClassName(val->enum_type(), true);
      break;
    default:
      vars["val_cpp"] = PrimitiveTypeName(val->cpp_type());
  }
  vars["key_wire_type"] = "::google::protobuf::internal::WireFormatLite::TYPE_" +
                          ToUpper(DeclaredTypeMethodName(key->type()));
  vars["val_wire_type"] = "::google::protobuf::internal::WireFormatLite::TYPE_" +
                          ToUpper(DeclaredTypeMethodName(val->type()));
  if (descriptor->file()->syntax() != FileDescriptor::SYNTAX_PROTO3 &&
      val->type() == FieldDescriptor::TYPE_ENUM) {
    const EnumValueDescriptor* default_value = val->default_value_enum();
    vars["default_enum_value"] = Int32ToString(default_value->number());
  } else {
    vars["default_enum_value"] = "0";
  }
}

// Does the given field have a private (internal helper only) has_$name$()
// method?
bool HasPrivateHasMethod(const FieldDescriptor* field) {
  // Only for oneofs in message types with no field presence. has_$name$(),
  // based on the oneof case, is still useful internally for generated code.
  return (!HasFieldPresence(field->file()) &&
          field->containing_oneof() != NULL);
}


bool TableDrivenParsingEnabled(
    const Descriptor* descriptor, const Options& options) {
  if (!options.table_driven_parsing) {
    return false;
  }

  // Consider table-driven parsing.  We only do this if:
  // - We have has_bits for fields.  This avoids a check on every field we set
  //   when are present (the common case).
  if (!HasFieldPresence(descriptor->file())) {
    return false;
  }

  const double table_sparseness = 0.5;
  int max_field_number = 0;
  for (int i = 0; i < descriptor->field_count(); i++) {
    const FieldDescriptor* field = descriptor->field(i);
    if (max_field_number < field->number()) {
      max_field_number = field->number();
    }

    // - There are no weak fields.
    if (field->options().weak()) {
      return false;
    }
  }

  // - There range of field numbers is "small"
  if (max_field_number >= (2 << 14)) {
    return false;
  }

  // - Field numbers are relatively dense within the actual number of fields.
  //   We check for strictly greater than in the case where there are no fields
  //   (only extensions) so max_field_number == descriptor->field_count() == 0.
  if (max_field_number * table_sparseness > descriptor->field_count()) {
    return false;
  }

  // - This is not a MapEntryMessage.
  if (IsMapEntryMessage(descriptor)) {
    return false;
  }

  return true;
}

void SetUnknkownFieldsVariable(const Descriptor* descriptor,
                               const Options& options,
                               std::map<string, string>* variables) {
  if (UseUnknownFieldSet(descriptor->file(), options)) {
    (*variables)["unknown_fields_type"] = "::google::protobuf::UnknownFieldSet";
  } else {
    (*variables)["unknown_fields_type"] = "::std::string";
  }
  if (AlwaysPreserveUnknownFields(descriptor)) {
    (*variables)["have_unknown_fields"] =
        "_internal_metadata_.have_unknown_fields()";
    (*variables)["unknown_fields"] = "_internal_metadata_.unknown_fields()";
  } else {
    (*variables)["have_unknown_fields"] =
        "(_internal_metadata_.have_unknown_fields() && "
        " ::google::protobuf::internal::GetProto3PreserveUnknownsDefault())";
    (*variables)["unknown_fields"] =
        "(::google::protobuf::internal::GetProto3PreserveUnknownsDefault()"
        "   ? _internal_metadata_.unknown_fields()"
        "   : _internal_metadata_.default_instance())";
  }
  (*variables)["mutable_unknown_fields"] =
      "_internal_metadata_.mutable_unknown_fields()";
}

bool IsCrossFileMapField(const FieldDescriptor* field) {
  if (!field->is_map()) {
    return false;
  }

  const Descriptor* d = field->message_type();
  const FieldDescriptor* value = d->FindFieldByNumber(2);

  return IsCrossFileMessage(value);
}

bool IsCrossFileMaybeMap(const FieldDescriptor* field) {
  if (IsCrossFileMapField(field)) {
    return true;
  }

  return IsCrossFileMessage(field);
}

bool IsRequired(const std::vector<const FieldDescriptor*>& v) {
  return v.front()->is_required();
}

// Allows chunking repeated fields together and non-repeated fields if the
// fields share the same has_byte index.
// TODO(seongkim): use lambda with capture instead of functor.
class MatchRepeatedAndHasByte {
 public:
  MatchRepeatedAndHasByte(const std::vector<int>* has_bit_indices,
                          bool has_field_presence)
      : has_bit_indices_(*has_bit_indices),
        has_field_presence_(has_field_presence) {}

  // Returns true if the following conditions are met:
  // --both fields are repeated fields
  // --both fields are non-repeated fields with either has_field_presence is
  //   false or have the same has_byte index.
  bool operator()(const FieldDescriptor* a, const FieldDescriptor* b) const {
    return a->is_repeated() == b->is_repeated() &&
           (!has_field_presence_ || a->is_repeated() ||
            has_bit_indices_[a->index()] / 8 ==
                has_bit_indices_[b->index()] / 8);
  }

 private:
  const std::vector<int>& has_bit_indices_;
  const bool has_field_presence_;
};

// Allows chunking required fields separately after chunking with
// MatchRepeatedAndHasByte.
class MatchRepeatedAndHasByteAndRequired : public MatchRepeatedAndHasByte {
 public:
  MatchRepeatedAndHasByteAndRequired(const std::vector<int>* has_bit_indices,
                                     bool has_field_presence)
      : MatchRepeatedAndHasByte(has_bit_indices, has_field_presence) {}

  bool operator()(const FieldDescriptor* a, const FieldDescriptor* b) const {
    return MatchRepeatedAndHasByte::operator()(a, b) &&
           a->is_required() == b->is_required();
  }
};

// Allows chunking zero-initializable fields separately after chunking with
// MatchRepeatedAndHasByte.
class MatchRepeatedAndHasByteAndZeroInits : public MatchRepeatedAndHasByte {
 public:
  MatchRepeatedAndHasByteAndZeroInits(const std::vector<int>* has_bit_indices,
                                      bool has_field_presence)
      : MatchRepeatedAndHasByte(has_bit_indices, has_field_presence) {}

  bool operator()(const FieldDescriptor* a, const FieldDescriptor* b) const {
    return MatchRepeatedAndHasByte::operator()(a, b) &&
           CanInitializeByZeroing(a) == CanInitializeByZeroing(b);
  }
};

// Collects neighboring fields based on a given criteria (equivalent predicate).
template <typename Predicate>
std::vector<std::vector<const FieldDescriptor*> > CollectFields(
    const std::vector<const FieldDescriptor*>& fields,
    const Predicate& equivalent) {
  std::vector<std::vector<const FieldDescriptor*> > chunks;
  if (fields.empty()) {
    return chunks;
  }

  const FieldDescriptor* last_field = fields.front();
  std::vector<const FieldDescriptor*> chunk;
  for (int i = 0; i < fields.size(); i++) {
    if (!equivalent(last_field, fields[i]) && !chunk.empty()) {
      chunks.push_back(chunk);
      chunk.clear();
    }
    chunk.push_back(fields[i]);
    last_field = fields[i];
  }
  if (!chunk.empty()) {
    chunks.push_back(chunk);
  }
  return chunks;
}

// Returns a bit mask based on has_bit index of "fields" that are typically on
// the same chunk. It is used in a group presence check where _has_bits_ is
// masked to tell if any thing in "fields" is present.
uint32 GenChunkMask(const std::vector<const FieldDescriptor*>& fields,
                    const std::vector<int>& has_bit_indices) {
  GOOGLE_CHECK(!fields.empty());
  int first_index_offset = has_bit_indices[fields.front()->index()] / 32;
  uint32 chunk_mask = 0;
  for (int i = 0; i < fields.size(); i++) {
    const FieldDescriptor* field = fields[i];
    // "index" defines where in the _has_bits_ the field appears.
    int index = has_bit_indices[field->index()];
    GOOGLE_CHECK_EQ(first_index_offset, index / 32);
    chunk_mask |= static_cast<uint32>(1) << (index % 32);
  }
  GOOGLE_CHECK_NE(0, chunk_mask);
  return chunk_mask;
}

}  // anonymous namespace

// ===================================================================

MessageGenerator::MessageGenerator(const Descriptor* descriptor,
                                   int index_in_file_messages,
                                   const Options& options,
                                   SCCAnalyzer* scc_analyzer)
    : descriptor_(descriptor),
      index_in_file_messages_(index_in_file_messages),
      classname_(ClassName(descriptor, false)),
      options_(options),
      field_generators_(descriptor, options, scc_analyzer),
      max_has_bit_index_(0),
      enum_generators_(
          new std::unique_ptr<EnumGenerator>[descriptor->enum_type_count()]),
      extension_generators_(new std::unique_ptr<
                            ExtensionGenerator>[descriptor->extension_count()]),
      num_weak_fields_(0),
      message_layout_helper_(new PaddingOptimizer()),
      scc_analyzer_(scc_analyzer) {
  // Compute optimized field order to be used for layout and initialization
  // purposes.
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (field->options().weak()) {
      num_weak_fields_++;
    } else if (!field->containing_oneof()) {
      optimized_order_.push_back(field);
    }
  }

  message_layout_helper_->OptimizeLayout(&optimized_order_, options_);

  if (HasFieldPresence(descriptor_->file())) {
    // We use -1 as a sentinel.
    has_bit_indices_.resize(descriptor_->field_count(), -1);
    for (int i = 0; i < optimized_order_.size(); i++) {
      const FieldDescriptor* field = optimized_order_[i];
      // Skip fields that do not have has bits.
      if (field->is_repeated()) {
        continue;
      }

      has_bit_indices_[field->index()] = max_has_bit_index_++;
    }
  }

  for (int i = 0; i < descriptor->enum_type_count(); i++) {
    enum_generators_[i].reset(
      new EnumGenerator(descriptor->enum_type(i), options));
  }

  for (int i = 0; i < descriptor->extension_count(); i++) {
    extension_generators_[i].reset(
      new ExtensionGenerator(descriptor->extension(i), options));
  }

  num_required_fields_ = 0;
  for (int i = 0; i < descriptor->field_count(); i++) {
    if (descriptor->field(i)->is_required()) {
      ++num_required_fields_;
    }
  }

  table_driven_ = TableDrivenParsingEnabled(descriptor_, options_);

  scc_name_ =
      ClassName(scc_analyzer_->GetSCC(descriptor_)->GetRepresentative(), false);
}

MessageGenerator::~MessageGenerator() {}

size_t MessageGenerator::HasBitsSize() const {
  size_t sizeof_has_bits = (max_has_bit_index_ + 31) / 32 * 4;
  if (sizeof_has_bits == 0) {
    // Zero-size arrays aren't technically allowed, and MSVC in particular
    // doesn't like them.  We still need to declare these arrays to make
    // other code compile.  Since this is an uncommon case, we'll just declare
    // them with size 1 and waste some space.  Oh well.
    sizeof_has_bits = 4;
  }

  return sizeof_has_bits;
}

void MessageGenerator::AddGenerators(
    std::vector<EnumGenerator*>* enum_generators,
    std::vector<ExtensionGenerator*>* extension_generators) {
  for (int i = 0; i < descriptor_->enum_type_count(); i++) {
    enum_generators->push_back(enum_generators_[i].get());
  }
  for (int i = 0; i < descriptor_->extension_count(); i++) {
    extension_generators->push_back(extension_generators_[i].get());
  }
}

void MessageGenerator::FillMessageForwardDeclarations(
    std::map<string, const Descriptor*>* class_names) {
  (*class_names)[classname_] = descriptor_;
}

void MessageGenerator::
GenerateFieldAccessorDeclarations(io::Printer* printer) {
  // optimized_fields_ does not contain fields where
  //    field->containing_oneof() != NULL
  // so we need to iterate over those as well.
  //
  // We place the non-oneof fields in optimized_order_, as that controls the
  // order of the _has_bits_ entries and we want GDB's pretty printers to be
  // able to infer these indices from the k[FIELDNAME]FieldNumber order.
  std::vector<const FieldDescriptor*> ordered_fields;
  ordered_fields.reserve(descriptor_->field_count());

  ordered_fields.insert(
      ordered_fields.begin(), optimized_order_.begin(), optimized_order_.end());
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (field->containing_oneof() == NULL && !field->options().weak()) {
      continue;
    }
    ordered_fields.push_back(field);
  }

  for (int i = 0; i < ordered_fields.size(); i++) {
    const FieldDescriptor* field = ordered_fields[i];

    PrintFieldComment(printer, field);

    std::map<string, string> vars;
    SetCommonFieldVariables(field, &vars, options_);
    vars["constant_name"] = FieldConstantName(field);

    if (field->is_repeated()) {
      printer->Print(vars, "$deprecated_attr$int ${$$name$_size$}$() const;\n");
      printer->Annotate("{", "}", field);
    } else if (HasHasMethod(field)) {
      printer->Print(vars, "$deprecated_attr$bool ${$has_$name$$}$() const;\n");
      printer->Annotate("{", "}", field);
    } else if (HasPrivateHasMethod(field)) {
      printer->Print(vars,
                     "private:\n"
                     "bool ${$has_$name$$}$() const;\n"
                     "public:\n");
      printer->Annotate("{", "}", field);
    }

    printer->Print(vars, "$deprecated_attr$void ${$clear_$name$$}$();\n");
    printer->Annotate("{", "}", field);
    printer->Print(vars,
                   "$deprecated_attr$static const int $constant_name$ = "
                   "$number$;\n");
    printer->Annotate("constant_name", field);

    // Generate type-specific accessor declarations.
    field_generators_.get(field).GenerateAccessorDeclarations(printer);

    printer->Print("\n");
  }

  if (descriptor_->extension_range_count() > 0) {
    // Generate accessors for extensions.  We just call a macro located in
    // extension_set.h since the accessors about 80 lines of static code.
    printer->Print(
      "GOOGLE_PROTOBUF_EXTENSION_ACCESSORS($classname$)\n",
      "classname", classname_);
  }

  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "void clear_$oneof_name$();\n"
        "$camel_oneof_name$Case $oneof_name$_case() const;\n",
        "camel_oneof_name",
        UnderscoresToCamelCase(descriptor_->oneof_decl(i)->name(), true),
        "oneof_name", descriptor_->oneof_decl(i)->name());
  }
}

void MessageGenerator::
GenerateSingularFieldHasBits(const FieldDescriptor* field,
                             std::map<string, string> vars,
                             io::Printer* printer) {
  if (field->options().weak()) {
    printer->Print(
        vars,
        "inline bool $classname$::has_$name$() const {\n"
        "  return _weak_field_map_.Has($number$);\n"
        "}\n");
    return;
  }
  if (HasFieldPresence(descriptor_->file())) {
    // N.B.: without field presence, we do not use has-bits or generate
    // has_$name$() methods.
    int has_bit_index = has_bit_indices_[field->index()];
    GOOGLE_CHECK_GE(has_bit_index, 0);

    vars["has_array_index"] = SimpleItoa(has_bit_index / 32);
    vars["has_mask"] = StrCat(strings::Hex(1u << (has_bit_index % 32),
                                           strings::ZERO_PAD_8));
    printer->Print(vars,
      "inline bool $classname$::has_$name$() const {\n"
      "  return (_has_bits_[$has_array_index$] & 0x$has_mask$u) != 0;\n"
      "}\n"
      "inline void $classname$::set_has_$name$() {\n"
      "  _has_bits_[$has_array_index$] |= 0x$has_mask$u;\n"
      "}\n"
      "inline void $classname$::clear_has_$name$() {\n"
      "  _has_bits_[$has_array_index$] &= ~0x$has_mask$u;\n"
      "}\n");
  } else {
    // Message fields have a has_$name$() method.
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      bool is_lazy = false;
      if (is_lazy) {
        printer->Print(vars,
          "inline bool $classname$::has_$name$() const {\n"
          "  return !$name$_.IsCleared();\n"
          "}\n");
      } else {
        printer->Print(
            vars,
            "inline bool $classname$::has_$name$() const {\n"
            "  return this != internal_default_instance() && $name$_ != NULL;\n"
            "}\n");
      }
    }
  }
}

void MessageGenerator::
GenerateOneofHasBits(io::Printer* printer) {
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    std::map<string, string> vars;
    vars["oneof_name"] = descriptor_->oneof_decl(i)->name();
    vars["oneof_index"] = SimpleItoa(descriptor_->oneof_decl(i)->index());
    vars["cap_oneof_name"] =
        ToUpper(descriptor_->oneof_decl(i)->name());
    vars["classname"] = classname_;
    printer->Print(
        vars,
        "inline bool $classname$::has_$oneof_name$() const {\n"
        "  return $oneof_name$_case() != $cap_oneof_name$_NOT_SET;\n"
        "}\n"
        "inline void $classname$::clear_has_$oneof_name$() {\n"
        "  _oneof_case_[$oneof_index$] = $cap_oneof_name$_NOT_SET;\n"
        "}\n");
  }
}

void MessageGenerator::
GenerateOneofMemberHasBits(const FieldDescriptor* field,
                           const std::map<string, string>& vars,
                           io::Printer* printer) {
  // Singular field in a oneof
  // N.B.: Without field presence, we do not use has-bits or generate
  // has_$name$() methods, but oneofs still have set_has_$name$().
  // Oneofs also have has_$name$() but only as a private helper
  // method, so that generated code is slightly cleaner (vs.  comparing
  // _oneof_case_[index] against a constant everywhere).
  printer->Print(vars,
    "inline bool $classname$::has_$name$() const {\n"
    "  return $oneof_name$_case() == k$field_name$;\n"
    "}\n");
  printer->Print(vars,
    "inline void $classname$::set_has_$name$() {\n"
    "  _oneof_case_[$oneof_index$] = k$field_name$;\n"
    "}\n");
}

void MessageGenerator::
GenerateFieldClear(const FieldDescriptor* field,
                   const std::map<string, string>& vars,
                   bool is_inline,
                   io::Printer* printer) {
  // Generate clear_$name$().
  if (is_inline) {
    printer->Print("inline ");
  }
  printer->Print(vars,
    "void $classname$::clear_$name$() {\n");

  printer->Indent();

  if (field->containing_oneof()) {
    // Clear this field only if it is the active field in this oneof,
    // otherwise ignore
    printer->Print(vars,
      "if (has_$name$()) {\n");
    printer->Indent();
    field_generators_.get(field)
        .GenerateClearingCode(printer);
    printer->Print(vars,
      "clear_has_$oneof_name$();\n");
    printer->Outdent();
    printer->Print("}\n");
  } else {
    field_generators_.get(field)
        .GenerateClearingCode(printer);
    if (HasFieldPresence(descriptor_->file())) {
      if (!field->is_repeated() && !field->options().weak()) {
        printer->Print(vars, "clear_has_$name$();\n");
      }
    }
  }

  printer->Outdent();
  printer->Print("}\n");
}

void MessageGenerator::
GenerateFieldAccessorDefinitions(io::Printer* printer) {
  printer->Print("// $classname$\n\n", "classname", classname_);

  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);

    PrintFieldComment(printer, field);

    std::map<string, string> vars;
    SetCommonFieldVariables(field, &vars, options_);

    // Generate has_$name$() or $name$_size().
    if (field->is_repeated()) {
      printer->Print(vars,
        "inline int $classname$::$name$_size() const {\n"
        "  return $name$_.size();\n"
        "}\n");
    } else if (field->containing_oneof()) {
      vars["field_name"] = UnderscoresToCamelCase(field->name(), true);
      vars["oneof_name"] = field->containing_oneof()->name();
      vars["oneof_index"] = SimpleItoa(field->containing_oneof()->index());
      GenerateOneofMemberHasBits(field, vars, printer);
    } else {
      // Singular field.
      GenerateSingularFieldHasBits(field, vars, printer);
    }

    if (!IsCrossFileMaybeMap(field)) {
      GenerateFieldClear(field, vars, true, printer);
    }

    // Generate type-specific accessors.
    field_generators_.get(field).GenerateInlineAccessorDefinitions(printer);

    printer->Print("\n");
  }

  // Generate has_$name$() and clear_has_$name$() functions for oneofs.
  GenerateOneofHasBits(printer);
}

void MessageGenerator::
GenerateClassDefinition(io::Printer* printer) {
  if (IsMapEntryMessage(descriptor_)) {
    std::map<string, string> vars;
    vars["classname"] = classname_;
    CollectMapInfo(descriptor_, &vars);
    vars["lite"] =
        HasDescriptorMethods(descriptor_->file(), options_) ? "" : "Lite";
    printer->Print(
        vars,
        "class $classname$ : public "
        "::google::protobuf::internal::MapEntry$lite$<$classname$, \n"
        "    $key_cpp$, $val_cpp$,\n"
        "    $key_wire_type$,\n"
        "    $val_wire_type$,\n"
        "    $default_enum_value$ > {\n"
        "public:\n"
        "  typedef ::google::protobuf::internal::MapEntry$lite$<$classname$, \n"
        "    $key_cpp$, $val_cpp$,\n"
        "    $key_wire_type$,\n"
        "    $val_wire_type$,\n"
        "    $default_enum_value$ > SuperType;\n"
        "  $classname$();\n"
        "  $classname$(::google::protobuf::Arena* arena);\n"
        "  void MergeFrom(const $classname$& other);\n"
        "  static const $classname$* internal_default_instance() { return "
        "reinterpret_cast<const "
        "$classname$*>(&_$classname$_default_instance_); }\n");
    if (HasDescriptorMethods(descriptor_->file(), options_)) {
      printer->Print(
          "  void MergeFrom(const ::google::protobuf::Message& other) final;\n"
          "  ::google::protobuf::Metadata GetMetadata() const;\n"
          "};\n");
    } else {
      printer->Print("};\n");
    }
    return;
  }

  std::map<string, string> vars;
  vars["classname"] = classname_;
  vars["full_name"] = descriptor_->full_name();
  vars["field_count"] = SimpleItoa(descriptor_->field_count());
  vars["oneof_decl_count"] = SimpleItoa(descriptor_->oneof_decl_count());
  if (options_.dllexport_decl.empty()) {
    vars["dllexport"] = "";
  } else {
    vars["dllexport"] = options_.dllexport_decl + " ";
  }
  vars["superclass"] = SuperClassName(descriptor_, options_);
  printer->Print(vars,
    "class $dllexport$$classname$ : public $superclass$ "
    "/* @@protoc_insertion_point(class_definition:$full_name$) */ "
    "{\n");
  printer->Annotate("classname", descriptor_);
  printer->Print(" public:\n");
  printer->Indent();

  printer->Print(
      vars,
      "$classname$();\n"
      "virtual ~$classname$();\n"
      "\n"
      "$classname$(const $classname$& from);\n"
      "\n"
      "inline $classname$& operator=(const $classname$& from) {\n"
      "  CopyFrom(from);\n"
      "  return *this;\n"
      "}\n");

  if (options_.table_driven_serialization) {
    printer->Print(
      "private:\n"
      "const void* InternalGetTable() const;\n"
      "public:\n"
      "\n");
  }

  // Generate move constructor and move assignment operator.
  printer->Print(vars,
    "#if LANG_CXX11\n"
    "$classname$($classname$&& from) noexcept\n"
    "  : $classname$() {\n"
    "  *this = ::std::move(from);\n"
    "}\n"
    "\n"
    "inline $classname$& operator=($classname$&& from) noexcept {\n"
    "  if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {\n"
    "    if (this != &from) InternalSwap(&from);\n"
    "  } else {\n"
    "    CopyFrom(from);\n"
    "  }\n"
    "  return *this;\n"
    "}\n"
    "#endif\n");

  SetUnknkownFieldsVariable(descriptor_, options_, &vars);
  if (PublicUnknownFieldsAccessors(descriptor_)) {
    printer->Print(vars,
        "inline const $unknown_fields_type$& unknown_fields() const {\n"
        "  return $unknown_fields$;\n"
        "}\n"
        "inline $unknown_fields_type$* mutable_unknown_fields() {\n"
        "  return $mutable_unknown_fields$;\n"
        "}\n"
        "\n");
  }

  // N.B.: We exclude GetArena() when arena support is disabled, falling back on
  // MessageLite's implementation which returns NULL rather than generating our
  // own method which returns NULL, in order to reduce code size.
  if (SupportsArenas(descriptor_)) {
    // virtual method version of GetArenaNoVirtual(), required for generic dispatch given a
    // MessageLite* (e.g., in RepeatedField::AddAllocated()).
    printer->Print(
        "inline ::google::protobuf::Arena* GetArena() const final {\n"
        "  return GetArenaNoVirtual();\n"
        "}\n"
        "inline void* GetMaybeArenaPointer() const final {\n"
        "  return MaybeArenaPtr();\n"
        "}\n");
  }

  // Only generate this member if it's not disabled.
  if (HasDescriptorMethods(descriptor_->file(), options_) &&
      !descriptor_->options().no_standard_descriptor_accessor()) {
    printer->Print(vars,
      "static const ::google::protobuf::Descriptor* descriptor();\n");
  }

  printer->Print(vars,
    "static const $classname$& default_instance();\n"
    "\n");

  // Generate enum values for every field in oneofs. One list is generated for
  // each oneof with an additional *_NOT_SET value.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "enum $camel_oneof_name$Case {\n",
        "camel_oneof_name",
        UnderscoresToCamelCase(descriptor_->oneof_decl(i)->name(), true));
    printer->Indent();
    for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
      printer->Print(
          "k$field_name$ = $field_number$,\n",
          "field_name",
          UnderscoresToCamelCase(
              descriptor_->oneof_decl(i)->field(j)->name(), true),
          "field_number",
          SimpleItoa(descriptor_->oneof_decl(i)->field(j)->number()));
    }
    printer->Print(
        "$cap_oneof_name$_NOT_SET = 0,\n",
        "cap_oneof_name",
        ToUpper(descriptor_->oneof_decl(i)->name()));
    printer->Outdent();
    printer->Print(
        "};\n"
        "\n");
  }

  // TODO(gerbens) make this private, while still granting other protos access.
  vars["message_index"] = SimpleItoa(index_in_file_messages_);
  printer->Print(
      vars,
      "static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY\n"
      "static inline const $classname$* internal_default_instance() {\n"
      "  return reinterpret_cast<const $classname$*>(\n"
      "             &_$classname$_default_instance_);\n"
      "}\n"
      "static constexpr int kIndexInFileMessages =\n"
      "  $message_index$;\n"
      "\n");

  if (SupportsArenas(descriptor_)) {
    printer->Print(vars,
      "void UnsafeArenaSwap($classname$* other);\n");
  }

  if (IsAnyMessage(descriptor_)) {
    printer->Print(vars,
      "// implements Any -----------------------------------------------\n"
      "\n"
      "void PackFrom(const ::google::protobuf::Message& message);\n"
      "void PackFrom(const ::google::protobuf::Message& message,\n"
      "              const ::std::string& type_url_prefix);\n"
      "bool UnpackTo(::google::protobuf::Message* message) const;\n"
      "template<typename T> bool Is() const {\n"
      "  return _any_metadata_.Is<T>();\n"
      "}\n"
      "static bool ParseAnyTypeUrl(const string& type_url,\n"
      "                            string* full_type_name);\n"
      "\n");
  }

  vars["new_final"] = " final";

  printer->Print(vars,
    "void Swap($classname$* other);\n"
    "friend void swap($classname$& a, $classname$& b) {\n"
    "  a.Swap(&b);\n"
    "}\n"
    "\n"
    "// implements Message ----------------------------------------------\n"
    "\n"
    "inline $classname$* New() const$new_final$ {\n"
    "  return CreateMaybeMessage<$classname$>(NULL);\n"
    "}\n"
    "\n"
    "$classname$* New(::google::protobuf::Arena* arena) const$new_final$ {\n"
    "  return CreateMaybeMessage<$classname$>(arena);\n"
    "}\n");

  // For instances that derive from Message (rather than MessageLite), some
  // methods are virtual and should be marked as final.
  string use_final = HasDescriptorMethods(descriptor_->file(), options_) ?
      " final" : "";

  if (HasGeneratedMethods(descriptor_->file(), options_)) {
    if (HasDescriptorMethods(descriptor_->file(), options_)) {
      printer->Print(vars,
        "void CopyFrom(const ::google::protobuf::Message& from) final;\n"
        "void MergeFrom(const ::google::protobuf::Message& from) final;\n");
    } else {
      printer->Print(vars,
        "void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from)\n"
        "  final;\n");
    }

    vars["clear_final"] = " final";
    vars["is_initialized_final"] = " final";
    vars["merge_partial_final"] = " final";

    printer->Print(
        vars,
        "void CopyFrom(const $classname$& from);\n"
        "void MergeFrom(const $classname$& from);\n"
        "void Clear()$clear_final$;\n"
        "bool IsInitialized() const$is_initialized_final$;\n"
        "\n"
        "size_t ByteSizeLong() const final;\n"
        "bool MergePartialFromCodedStream(\n"
        "    ::google::protobuf::io::CodedInputStream* input)$merge_partial_final$;\n");
    if (!options_.table_driven_serialization ||
        descriptor_->options().message_set_wire_format()) {
      printer->Print(
          "void SerializeWithCachedSizes(\n"
          "    ::google::protobuf::io::CodedOutputStream* output) const "
          "final;\n");
    }
    // DiscardUnknownFields() is implemented in message.cc using reflections. We
    // need to implement this function in generated code for messages.
    if (!UseUnknownFieldSet(descriptor_->file(), options_)) {
      printer->Print(
        "void DiscardUnknownFields()$final$;\n",
        "final", use_final);
    }
    if (HasFastArraySerialization(descriptor_->file(), options_)) {
      printer->Print(
        "::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(\n"
        "    bool deterministic, ::google::protobuf::uint8* target) const final;\n");
    }
  }

  printer->Print(
      "int GetCachedSize() const final { return _cached_size_.Get(); }"
      "\n\nprivate:\n"
      "void SharedCtor();\n"
      "void SharedDtor();\n"
      "void SetCachedSize(int size) const$final$;\n"
      "void InternalSwap($classname$* other);\n",
      "classname", classname_, "final", use_final);
  if (SupportsArenas(descriptor_)) {
    printer->Print(
      // TODO(gerbens) Make this private! Currently people are deriving from
      // protos to give access to this constructor, breaking the invariants
      // we rely on.
      "protected:\n"
      "explicit $classname$(::google::protobuf::Arena* arena);\n"
      "private:\n"
      "static void ArenaDtor(void* object);\n"
      "inline void RegisterArenaDtor(::google::protobuf::Arena* arena);\n",
      "classname", classname_);
  }

  if (SupportsArenas(descriptor_)) {
    printer->Print(
      "private:\n"
      "inline ::google::protobuf::Arena* GetArenaNoVirtual() const {\n"
      "  return _internal_metadata_.arena();\n"
      "}\n"
      "inline void* MaybeArenaPtr() const {\n"
      "  return _internal_metadata_.raw_arena_ptr();\n"
      "}\n");
  } else {
    printer->Print(
      "private:\n"
      "inline ::google::protobuf::Arena* GetArenaNoVirtual() const {\n"
      "  return NULL;\n"
      "}\n"
      "inline void* MaybeArenaPtr() const {\n"
      "  return NULL;\n"
      "}\n");
  }

  printer->Print(
      "public:\n"
      "\n");

  if (HasDescriptorMethods(descriptor_->file(), options_)) {
    printer->Print(
      "::google::protobuf::Metadata GetMetadata() const final;\n"
      "\n");
  } else {
    printer->Print(
      "::std::string GetTypeName() const final;\n"
      "\n");
  }

  printer->Print(
    "// nested types ----------------------------------------------------\n"
    "\n");

  // Import all nested message classes into this class's scope with typedefs.
  for (int i = 0; i < descriptor_->nested_type_count(); i++) {
    const Descriptor* nested_type = descriptor_->nested_type(i);
    if (!IsMapEntryMessage(nested_type)) {
      printer->Print("typedef $nested_full_name$ $nested_name$;\n",
                     "nested_name", nested_type->name(),
                     "nested_full_name", ClassName(nested_type, false));
      printer->Annotate("nested_full_name", nested_type);
      printer->Annotate("nested_name", nested_type);
    }
  }

  if (descriptor_->nested_type_count() > 0) {
    printer->Print("\n");
  }

  // Import all nested enums and their values into this class's scope with
  // typedefs and constants.
  for (int i = 0; i < descriptor_->enum_type_count(); i++) {
    enum_generators_[i]->GenerateSymbolImports(printer);
    printer->Print("\n");
  }

  printer->Print(
    "// accessors -------------------------------------------------------\n"
    "\n");

  // Generate accessor methods for all fields.
  GenerateFieldAccessorDeclarations(printer);

  // Declare extension identifiers.
  for (int i = 0; i < descriptor_->extension_count(); i++) {
    extension_generators_[i]->GenerateDeclaration(printer);
  }


  printer->Print(
    "// @@protoc_insertion_point(class_scope:$full_name$)\n",
    "full_name", descriptor_->full_name());

  // Generate private members.
  printer->Outdent();
  printer->Print(" private:\n");
  printer->Indent();


  for (int i = 0; i < descriptor_->field_count(); i++) {
    if (!descriptor_->field(i)->is_repeated() &&
        !descriptor_->field(i)->options().weak()) {
      // set_has_***() generated in all proto1/2 code and in oneofs (only) for
      // messages without true field presence.
      if (HasFieldPresence(descriptor_->file()) ||
          descriptor_->field(i)->containing_oneof()) {
        printer->Print("void set_has_$name$();\n", "name",
                       FieldName(descriptor_->field(i)));
      }
      // clear_has_***() generated only for non-oneof fields
      // in proto1/2.
      if (!descriptor_->field(i)->containing_oneof() &&
          HasFieldPresence(descriptor_->file())) {
        printer->Print("void clear_has_$name$();\n", "name",
                       FieldName(descriptor_->field(i)));
      }
    }
  }
  printer->Print("\n");

  // Generate oneof function declarations
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "inline bool has_$oneof_name$() const;\n"
        "inline void clear_has_$oneof_name$();\n\n",
        "oneof_name", descriptor_->oneof_decl(i)->name());
  }

  if (HasGeneratedMethods(descriptor_->file(), options_) &&
      !descriptor_->options().message_set_wire_format() &&
      num_required_fields_ > 1) {
    printer->Print(
        "// helper for ByteSizeLong()\n"
        "size_t RequiredFieldsByteSizeFallback() const;\n\n");
  }

  // Prepare decls for _cached_size_ and _has_bits_.  Their position in the
  // output will be determined later.

  bool need_to_emit_cached_size = true;
  // TODO(kenton):  Make _cached_size_ an atomic<int> when C++ supports it.
  const string cached_size_decl =
      "mutable ::google::protobuf::internal::CachedSize _cached_size_;\n";

  const size_t sizeof_has_bits = HasBitsSize();
  const string has_bits_decl = sizeof_has_bits == 0 ? "" :
      "::google::protobuf::internal::HasBits<" + SimpleItoa(sizeof_has_bits / 4) +
      "> _has_bits_;\n";

  // To minimize padding, data members are divided into three sections:
  // (1) members assumed to align to 8 bytes
  // (2) members corresponding to message fields, re-ordered to optimize
  //     alignment.
  // (3) members assumed to align to 4 bytes.

  // Members assumed to align to 8 bytes:

  if (descriptor_->extension_range_count() > 0) {
    printer->Print(
      "::google::protobuf::internal::ExtensionSet _extensions_;\n"
      "\n");
  }

  if (UseUnknownFieldSet(descriptor_->file(), options_)) {
    printer->Print(
      "::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;\n");
  } else {
    printer->Print(
        "::google::protobuf::internal::InternalMetadataWithArenaLite "
        "_internal_metadata_;\n");
  }

  if (SupportsArenas(descriptor_)) {
    printer->Print(
      "template <typename T> friend class ::google::protobuf::Arena::InternalHelper;\n"
      "typedef void InternalArenaConstructable_;\n"
      "typedef void DestructorSkippable_;\n");
  }

  if (HasFieldPresence(descriptor_->file())) {
    // _has_bits_ is frequently accessed, so to reduce code size and improve
    // speed, it should be close to the start of the object.  But, try not to
    // waste space:_has_bits_ by itself always makes sense if its size is a
    // multiple of 8, but, otherwise, maybe _has_bits_ and cached_size_ together
    // will work well.
    printer->Print(has_bits_decl.c_str());
    if ((sizeof_has_bits % 8) != 0) {
      printer->Print(cached_size_decl.c_str());
      need_to_emit_cached_size = false;
    }
  }

  // Field members:

  // Emit some private and static members
  for (int i = 0; i < optimized_order_.size(); ++i) {
    const FieldDescriptor* field = optimized_order_[i];
    const FieldGenerator& generator = field_generators_.get(field);
    generator.GenerateStaticMembers(printer);
    generator.GeneratePrivateMembers(printer);
  }

  // For each oneof generate a union
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "union $camel_oneof_name$Union {\n"
        // explicit empty constructor is needed when union contains
        // ArenaStringPtr members for string fields.
        "  $camel_oneof_name$Union() {}\n",
        "camel_oneof_name",
        UnderscoresToCamelCase(descriptor_->oneof_decl(i)->name(), true));
    printer->Indent();
    for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
      field_generators_.get(descriptor_->oneof_decl(i)->
                            field(j)).GeneratePrivateMembers(printer);
    }
    printer->Outdent();
    printer->Print(
        "} $oneof_name$_;\n",
        "oneof_name", descriptor_->oneof_decl(i)->name());
    for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
      field_generators_.get(descriptor_->oneof_decl(i)->
                            field(j)).GenerateStaticMembers(printer);
    }
  }

  // Members assumed to align to 4 bytes:

  if (need_to_emit_cached_size) {
    printer->Print(cached_size_decl.c_str());
    need_to_emit_cached_size = false;
  }

  // Generate _oneof_case_.
  if (descriptor_->oneof_decl_count() > 0) {
    printer->Print(vars,
      "::google::protobuf::uint32 _oneof_case_[$oneof_decl_count$];\n"
      "\n");
  }

  if (num_weak_fields_) {
    printer->Print(
        "::google::protobuf::internal::WeakFieldMap _weak_field_map_;\n");
  }
  // Generate _any_metadata_ for the Any type.
  if (IsAnyMessage(descriptor_)) {
    printer->Print(vars,
      "::google::protobuf::internal::AnyMetadata _any_metadata_;\n");
  }

  // The TableStruct struct needs access to the private parts, in order to
  // construct the offsets of all members.
  printer->Print("friend struct ::$file_namespace$::TableStruct;\n",
                 // Vars.
                 "scc_name", scc_name_, "file_namespace",
                 FileLevelNamespace(descriptor_));

  printer->Outdent();
  printer->Print("};");
  GOOGLE_DCHECK(!need_to_emit_cached_size);
}

void MessageGenerator::
GenerateInlineMethods(io::Printer* printer) {
  if (IsMapEntryMessage(descriptor_)) return;
  GenerateFieldAccessorDefinitions(printer);

  // Generate oneof_case() functions.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    std::map<string, string> vars;
    vars["class_name"] = classname_;
    vars["camel_oneof_name"] = UnderscoresToCamelCase(
        descriptor_->oneof_decl(i)->name(), true);
    vars["oneof_name"] = descriptor_->oneof_decl(i)->name();
    vars["oneof_index"] = SimpleItoa(descriptor_->oneof_decl(i)->index());
    printer->Print(
        vars,
        "inline $class_name$::$camel_oneof_name$Case $class_name$::"
        "$oneof_name$_case() const {\n"
        "  return $class_name$::$camel_oneof_name$Case("
        "_oneof_case_[$oneof_index$]);\n"
        "}\n");
  }
}

void MessageGenerator::
GenerateExtraDefaultFields(io::Printer* printer) {
  // Generate oneof default instance and weak field instances for reflection
  // usage.
  if (descriptor_->oneof_decl_count() > 0 || num_weak_fields_ > 0) {
    for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
      for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
        const FieldDescriptor* field = descriptor_->oneof_decl(i)->field(j);
        if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE ||
            (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING &&
             EffectiveStringCType(field) != FieldOptions::STRING)) {
          printer->Print("const ");
        }
        field_generators_.get(field).GeneratePrivateMembers(printer);
      }
    }
    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor* field = descriptor_->field(i);
      if (field->options().weak()) {
        printer->Print(
            "  const ::google::protobuf::Message* $name$_;\n", "name", FieldName(field));
      }
    }
  }
}

bool MessageGenerator::GenerateParseTable(io::Printer* printer, size_t offset,
                                          size_t aux_offset) {
  if (!table_driven_) {
    printer->Print("{ NULL, NULL, 0, -1, -1, -1, -1, NULL, false },\n");
    return false;
  }

  std::map<string, string> vars;

  vars["classname"] = ClassName(descriptor_);
  vars["classtype"] = QualifiedClassName(descriptor_);
  vars["offset"] = SimpleItoa(offset);
  vars["aux_offset"] = SimpleItoa(aux_offset);

  int max_field_number = 0;
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (max_field_number < field->number()) {
      max_field_number = field->number();
    }
  }

  vars["max_field_number"] = SimpleItoa(max_field_number);

  printer->Print("{\n");
  printer->Indent();

  printer->Print(vars,
      "TableStruct::entries + $offset$,\n"
      "TableStruct::aux + $aux_offset$,\n"
      "$max_field_number$,\n");

  if (!HasFieldPresence(descriptor_->file())) {
    // If we don't have field presence, then _has_bits_ does not exist.
    printer->Print(vars, "-1,\n");
  } else {
    printer->Print(vars,
                   "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(\n"
                   "  $classtype$, _has_bits_),\n");
  }

  if (descriptor_->oneof_decl_count() > 0) {
    printer->Print(vars,
                   "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(\n"
                   "  $classtype$, _oneof_case_),\n");
  } else {
    printer->Print("-1,  // no _oneof_case_\n");
  }

  if (descriptor_->extension_range_count() > 0) {
    printer->Print(vars,
                   "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classtype$, "
                   "_extensions_),\n");
  } else {
    printer->Print("-1,  // no _extensions_\n");
  }

  // TODO(ckennelly): Consolidate this with the calculation for
  // AuxillaryParseTableField.
  vars["ns"] = Namespace(descriptor_);

  printer->Print(vars,
                 "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(\n"
                 "  $classtype$, _internal_metadata_),\n"
                 "&$ns$::_$classname$_default_instance_,\n");

  if (UseUnknownFieldSet(descriptor_->file(), options_)) {
    printer->Print(vars, "true,\n");
  } else {
    printer->Print(vars, "false,\n");
  }

  printer->Outdent();
  printer->Print("},\n");
  return true;
}

void MessageGenerator::GenerateSchema(io::Printer* printer, int offset,
                                      int has_offset) {
  std::map<string, string> vars;

  vars["classname"] = QualifiedClassName(descriptor_);
  vars["offset"] = SimpleItoa(offset);
  vars["has_bits_offsets"] =
      HasFieldPresence(descriptor_->file()) || IsMapEntryMessage(descriptor_)
          ? SimpleItoa(offset + has_offset)
          : "-1";

  printer->Print(vars,
                 "{ $offset$, $has_bits_offsets$, sizeof($classname$)},\n");
}

namespace {

// TODO(gerbens) remove this after the next sync with GitHub code base.
// Then the opensource testing has gained the functionality to compile
// the CalcFieldNum given the symbols defined in generated-message-util.
#ifdef OPENSOURCE_PROTOBUF_CPP_BOOTSTRAP
// We need a clean version of CalcFieldNum that doesn't use new functionality
// in the runtime, because this functionality is not yet in the opensource
// runtime

uint32 CalculateType(uint32 type, uint32 type_class) {
  return (type - 1) + type_class * 20;
}

uint32 CalcFieldNum(const FieldGenerator&, const FieldDescriptor* field,
                    const Options& options) {
  bool is_a_map = IsMapEntryMessage(field->containing_type());
  int type = field->type();
  if (field->containing_oneof()) {
    return CalculateType(type, 4);
  }
  if (field->is_packed()) {
    return CalculateType(type, 3);
  } else if (field->is_repeated()) {
    return CalculateType(type, 2);
  } else if (!HasFieldPresence(field->file()) &&
             field->containing_oneof() == NULL && !is_a_map) {
    return CalculateType(type, 1);
  } else {
    return CalculateType(type, 0);
  }
}

#else
// We need to calculate for each field what function the table driven code
// should use to serialize it. This returns the index in a lookup table.
uint32 CalcFieldNum(const FieldGenerator& generator,
                    const FieldDescriptor* field, const Options& options) {
  bool is_a_map = IsMapEntryMessage(field->containing_type());
  int type = field->type();
  if (type == FieldDescriptor::TYPE_STRING ||
      type == FieldDescriptor::TYPE_BYTES) {
    if (generator.IsInlined()) {
      type = internal::FieldMetadata::kInlinedType;
    }
  }
  if (field->containing_oneof()) {
    return internal::FieldMetadata::CalculateType(
        type, internal::FieldMetadata::kOneOf);
  }
  if (field->is_packed()) {
    return internal::FieldMetadata::CalculateType(
        type, internal::FieldMetadata::kPacked);
  } else if (field->is_repeated()) {
    return internal::FieldMetadata::CalculateType(
        type, internal::FieldMetadata::kRepeated);
  } else if (!HasFieldPresence(field->file()) &&
             field->containing_oneof() == NULL && !is_a_map) {
    return internal::FieldMetadata::CalculateType(
        type, internal::FieldMetadata::kNoPresence);
  } else {
    return internal::FieldMetadata::CalculateType(
        type, internal::FieldMetadata::kPresence);
  }
}
#endif

int FindMessageIndexInFile(const Descriptor* descriptor) {
  std::vector<const Descriptor*> flatten =
      FlattenMessagesInFile(descriptor->file());
  return std::find(flatten.begin(), flatten.end(), descriptor) -
         flatten.begin();
}

}  // namespace

int MessageGenerator::GenerateFieldMetadata(io::Printer* printer) {
  if (!options_.table_driven_serialization) {
    return 0;
  }

  string full_classname = QualifiedClassName(descriptor_);

  std::vector<const FieldDescriptor*> sorted = SortFieldsByNumber(descriptor_);
  if (IsMapEntryMessage(descriptor_)) {
    for (int i = 0; i < 2; i++) {
      const FieldDescriptor* field = sorted[i];
      const FieldGenerator& generator = field_generators_.get(field);

      uint32 tag = internal::WireFormatLite::MakeTag(
          field->number(), WireFormat::WireTypeForFieldType(field->type()));

      std::map<string, string> vars;
      vars["classname"] = QualifiedClassName(descriptor_);
      vars["field_name"] = FieldName(field);
      vars["tag"] = SimpleItoa(tag);
      vars["hasbit"] = SimpleItoa(i);
      vars["type"] = SimpleItoa(CalcFieldNum(generator, field, options_));
      vars["ptr"] = "NULL";
      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        GOOGLE_CHECK(!IsMapEntryMessage(field->message_type()));
        {
          vars["ptr"] =
              "::" + FileLevelNamespace(field->message_type()) +
              "::TableStruct::serialization_table + " +
              SimpleItoa(FindMessageIndexInFile(field->message_type()));
        }
      }
      printer->Print(vars,
                     "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET("
                     "::google::protobuf::internal::MapEntryHelper<$classname$::"
                     "SuperType>, $field_name$_), $tag$,"
                     "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET("
                     "::google::protobuf::internal::MapEntryHelper<$classname$::"
                     "SuperType>, _has_bits_) * 8 + $hasbit$, $type$, "
                     "$ptr$},\n");
    }
    return 2;
  }
  printer->Print(
      "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, "
      "_cached_size_), 0, 0, 0, NULL},\n",
      "classname", full_classname);
  std::vector<const Descriptor::ExtensionRange*> sorted_extensions;
  for (int i = 0; i < descriptor_->extension_range_count(); ++i) {
    sorted_extensions.push_back(descriptor_->extension_range(i));
  }
  std::sort(sorted_extensions.begin(), sorted_extensions.end(),
            ExtensionRangeSorter());
  for (int i = 0, extension_idx = 0; /* no range */; i++) {
    for (; extension_idx < sorted_extensions.size() &&
           (i == sorted.size() ||
            sorted_extensions[extension_idx]->start < sorted[i]->number());
         extension_idx++) {
      const Descriptor::ExtensionRange* range =
          sorted_extensions[extension_idx];
      printer->Print(
          "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, "
          "_extensions_), $start$, $end$, "
          "::google::protobuf::internal::FieldMetadata::kSpecial, "
          "reinterpret_cast<const "
          "void*>(::google::protobuf::internal::ExtensionSerializer)},\n",
          "classname", full_classname, "start", SimpleItoa(range->start), "end",
          SimpleItoa(range->end));
    }
    if (i == sorted.size()) break;
    const FieldDescriptor* field = sorted[i];

    uint32 tag = internal::WireFormatLite::MakeTag(
        field->number(), WireFormat::WireTypeForFieldType(field->type()));
    if (field->is_packed()) {
      tag = internal::WireFormatLite::MakeTag(
          field->number(), WireFormatLite::WIRETYPE_LENGTH_DELIMITED);
    }

    string classfieldname = FieldName(field);
    if (field->containing_oneof()) {
      classfieldname = field->containing_oneof()->name();
    }
    std::map<string, string> vars;
    vars["classname"] = full_classname;
    vars["field_name"] = classfieldname;
    vars["tag"] = SimpleItoa(tag);
    vars["ptr"] = "NULL";
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      if (IsMapEntryMessage(field->message_type())) {
        vars["idx"] = SimpleItoa(FindMessageIndexInFile(field->message_type()));
        vars["fieldclassname"] = QualifiedClassName(field->message_type());
        printer->Print(vars,
                       "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                       "classname$, $field_name$_), $tag$, $idx$, "
                       "::google::protobuf::internal::FieldMetadata::kSpecial, "
                       "reinterpret_cast<const void*>(static_cast< "
                       "::google::protobuf::internal::SpecialSerializer>("
                       "::google::protobuf::internal::MapFieldSerializer< "
                       "::google::protobuf::internal::MapEntryToMapField<"
                       "$fieldclassname$>::MapFieldType, "
                       "TableStruct::serialization_table>))},\n");
        continue;
      } else {
        vars["ptr"] =
            "::" + FileLevelNamespace(field->message_type()) +
            "::TableStruct::serialization_table + " +
            SimpleItoa(FindMessageIndexInFile(field->message_type()));
      }
    }

    const FieldGenerator& generator = field_generators_.get(field);
    vars["type"] = SimpleItoa(CalcFieldNum(generator, field, options_));


    if (field->options().weak()) {
      // TODO(gerbens) merge weak fields into ranges
      printer->Print(vars,
                     "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                     "classname$, _weak_field_map_), $tag$, $tag$, "
                     "::google::protobuf::internal::FieldMetadata::kSpecial, "
                     "reinterpret_cast<const "
                     "void*>(::google::protobuf::internal::WeakFieldSerializer)},\n");
    } else if (field->containing_oneof()) {
      vars["oneofoffset"] =
          SimpleItoa(sizeof(uint32) * field->containing_oneof()->index());
      printer->Print(vars,
                     "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                     "classname$, $field_name$_), $tag$, "
                     "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                     "classname$, _oneof_case_) + $oneofoffset$, "
                     "$type$, $ptr$},\n");
    } else if (HasFieldPresence(descriptor_->file()) &&
               has_bit_indices_[field->index()] != -1) {
      vars["hasbitsoffset"] = SimpleItoa(has_bit_indices_[field->index()]);
      printer->Print(vars,
                     "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                     "classname$, $field_name$_), $tag$, "
                     "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                     "classname$, _has_bits_) * 8 + $hasbitsoffset$, $type$, "
                     "$ptr$},\n");
    } else {
      printer->Print(vars,
                     "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($"
                     "classname$, $field_name$_), $tag$, ~0u, $type$, "
                     "$ptr$},\n");
    }
  }
  int num_field_metadata = 1 + sorted.size() + sorted_extensions.size();
  num_field_metadata++;
  string serializer = UseUnknownFieldSet(descriptor_->file(), options_)
                          ? "::google::protobuf::internal::UnknownFieldSetSerializer"
                          : "::google::protobuf::internal::UnknownFieldSerializerLite";
  printer->Print(
      "{GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, "
      "_internal_metadata_), 0, ~0u, "
      "::google::protobuf::internal::FieldMetadata::kSpecial, reinterpret_cast<const "
      "void*>($serializer$)},\n",
      "classname", full_classname, "serializer", serializer);
  return num_field_metadata;
}

void MessageGenerator::GenerateFieldDefaultInstances(io::Printer* printer) {
  // Construct the default instances for all fields that need one.
  for (int i = 0; i < descriptor_->field_count(); i++) {
    field_generators_.get(descriptor_->field(i))
                     .GenerateDefaultInstanceAllocator(printer);
  }
}

void MessageGenerator::
GenerateDefaultInstanceInitializer(io::Printer* printer) {
  // The default instance needs all of its embedded message pointers
  // cross-linked to other default instances.  We can't do this initialization
  // in the constructor because some other default instances may not have been
  // constructed yet at that time.
  // TODO(kenton):  Maybe all message fields (even for non-default messages)
  //   should be initialized to point at default instances rather than NULL?
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);

    if (!field->is_repeated() &&
        field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
        (field->containing_oneof() == NULL ||
         HasDescriptorMethods(descriptor_->file(), options_))) {
      string name;
      if (field->containing_oneof() || field->options().weak()) {
        name = "_" + classname_ + "_default_instance_.";
      } else {
        name =
            "_" + classname_ + "_default_instance_._instance.get_mutable()->";
      }
      name += FieldName(field);
      printer->Print(
          "$ns$::$name$_ = const_cast< $type$*>(\n"
          "    $type$::internal_default_instance());\n",
          // Vars.
          "name", name, "type", FieldMessageTypeName(field), "ns",
          Namespace(descriptor_));
    } else if (field->containing_oneof() &&
               HasDescriptorMethods(descriptor_->file(), options_)) {
      field_generators_.get(descriptor_->field(i))
          .GenerateConstructorCode(printer);
    }
  }
}

void MessageGenerator::
GenerateClassMethods(io::Printer* printer) {
  if (IsMapEntryMessage(descriptor_)) {
    printer->Print(
        "$classname$::$classname$() {}\n"
        "$classname$::$classname$(::google::protobuf::Arena* arena) : "
        "SuperType(arena) {}\n"
        "void $classname$::MergeFrom(const $classname$& other) {\n"
        "  MergeFromInternal(other);\n"
        "}\n",
        "classname", classname_);
    if (HasDescriptorMethods(descriptor_->file(), options_)) {
      printer->Print(
          "::google::protobuf::Metadata $classname$::GetMetadata() const {\n"
          "  ::$file_namespace$::protobuf_AssignDescriptorsOnce();\n"
          "  return ::$file_namespace$::file_level_metadata[$index$];\n"
          "}\n"
          "void $classname$::MergeFrom(\n"
          "    const ::google::protobuf::Message& other) {\n"
          "  ::google::protobuf::Message::MergeFrom(other);\n"
          "}\n"
          "\n",
          "file_namespace", FileLevelNamespace(descriptor_),
          "classname", classname_, "index",
          SimpleItoa(index_in_file_messages_));
    }
    return;
  }

  // TODO(gerbens) Remove this function. With a little bit of cleanup and
  // refactoring this is superfluous.
  printer->Print("void $classname$::InitAsDefaultInstance() {\n", "classname",
                 classname_);
  printer->Indent();
  GenerateDefaultInstanceInitializer(printer);
  printer->Outdent();
  printer->Print("}\n");

  if (IsAnyMessage(descriptor_)) {
    printer->Print(
      "void $classname$::PackFrom(const ::google::protobuf::Message& message) {\n"
      "  _any_metadata_.PackFrom(message);\n"
      "}\n"
      "\n"
      "void $classname$::PackFrom(const ::google::protobuf::Message& message,\n"
      "                           const ::std::string& type_url_prefix) {\n"
      "  _any_metadata_.PackFrom(message, type_url_prefix);\n"
      "}\n"
      "\n"
      "bool $classname$::UnpackTo(::google::protobuf::Message* message) const {\n"
      "  return _any_metadata_.UnpackTo(message);\n"
      "}\n"
      "bool $classname$::ParseAnyTypeUrl(const string& type_url,\n"
      "                                  string* full_type_name) {\n"
      "  return ::google::protobuf::internal::ParseAnyTypeUrl(type_url,\n"
      "                                             full_type_name);\n"
      "}\n"
      "\n",
      "classname", classname_);
  }

  // Generate non-inline field definitions.
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    field_generators_.get(field)
                     .GenerateNonInlineAccessorDefinitions(printer);
    if (IsCrossFileMaybeMap(field)) {
      std::map<string, string> vars;
      SetCommonFieldVariables(field, &vars, options_);
      if (field->containing_oneof()) {
        SetCommonOneofFieldVariables(field, &vars);
      }
      GenerateFieldClear(field, vars, false, printer);
    }
  }

  // Generate field number constants.
  printer->Print("#if !defined(_MSC_VER) || _MSC_VER >= 1900\n");
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor *field = descriptor_->field(i);
    printer->Print(
      "const int $classname$::$constant_name$;\n",
      "classname", ClassName(FieldScope(field), false),
      "constant_name", FieldConstantName(field));
  }
  printer->Print(
    "#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900\n"
    "\n");

  GenerateStructors(printer);
  printer->Print("\n");

  if (descriptor_->oneof_decl_count() > 0) {
    GenerateOneofClear(printer);
    printer->Print("\n");
  }

  if (HasGeneratedMethods(descriptor_->file(), options_)) {
    GenerateClear(printer);
    printer->Print("\n");

    GenerateMergeFromCodedStream(printer);
    printer->Print("\n");

    GenerateSerializeWithCachedSizes(printer);
    printer->Print("\n");

    if (HasFastArraySerialization(descriptor_->file(), options_)) {
      GenerateSerializeWithCachedSizesToArray(printer);
      printer->Print("\n");
    }

    GenerateByteSize(printer);
    printer->Print("\n");

    GenerateMergeFrom(printer);
    printer->Print("\n");

    GenerateCopyFrom(printer);
    printer->Print("\n");

    GenerateIsInitialized(printer);
    printer->Print("\n");
  }

  GenerateSwap(printer);
  printer->Print("\n");

  if (options_.table_driven_serialization) {
    printer->Print(
        "const void* $classname$::InternalGetTable() const {\n"
        "  return ::$file_namespace$::TableStruct::serialization_table + "
        "$index$;\n"
        "}\n"
        "\n",
        "classname", classname_, "index", SimpleItoa(index_in_file_messages_),
        "file_namespace", FileLevelNamespace(descriptor_));
  }
  if (HasDescriptorMethods(descriptor_->file(), options_)) {
    printer->Print(
        "::google::protobuf::Metadata $classname$::GetMetadata() const {\n"
        "  $file_namespace$::protobuf_AssignDescriptorsOnce();\n"
        "  return ::"
        "$file_namespace$::file_level_metadata[kIndexInFileMessages];\n"
        "}\n"
        "\n",
        "classname", classname_, "file_namespace",
        FileLevelNamespace(descriptor_));
  } else {
    printer->Print(
      "::std::string $classname$::GetTypeName() const {\n"
      "  return \"$type_name$\";\n"
      "}\n"
      "\n",
      "classname", classname_,
      "type_name", descriptor_->full_name());
  }

}

size_t MessageGenerator::GenerateParseOffsets(io::Printer* printer) {
  if (!table_driven_) {
    return 0;
  }

  // Field "0" is special:  We use it in our switch statement of processing
  // types to handle the successful end tag case.
  printer->Print("{0, 0, 0, ::google::protobuf::internal::kInvalidMask, 0, 0},\n");
  int last_field_number = 1;

  std::vector<const FieldDescriptor*> ordered_fields =
      SortFieldsByNumber(descriptor_);

  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = ordered_fields[i];
    GOOGLE_CHECK_GE(field->number(), last_field_number);

    for (; last_field_number < field->number(); last_field_number++) {
      printer->Print(
          "{ 0, 0, ::google::protobuf::internal::kInvalidMask,\n"
          "  ::google::protobuf::internal::kInvalidMask, 0, 0 },\n");
    }
    last_field_number++;

    unsigned char normal_wiretype, packed_wiretype, processing_type;
    normal_wiretype = WireFormat::WireTypeForFieldType(field->type());

    if (field->is_packable()) {
      packed_wiretype = WireFormatLite::WIRETYPE_LENGTH_DELIMITED;
    } else {
      packed_wiretype = internal::kNotPackedMask;
    }

    processing_type = static_cast<unsigned>(field->type());
    const FieldGenerator& generator = field_generators_.get(field);
    if (field->type() == FieldDescriptor::TYPE_STRING) {
      switch (EffectiveStringCType(field)) {
        case FieldOptions::STRING:
        default: {
          if (generator.IsInlined()) {
            processing_type = internal::TYPE_STRING_INLINED;
            break;
          }
          break;
        }
      }
    } else if (field->type() == FieldDescriptor::TYPE_BYTES) {
      switch (EffectiveStringCType(field)) {
        case FieldOptions::STRING:
        default:
          if (generator.IsInlined()) {
            processing_type = internal::TYPE_BYTES_INLINED;
            break;
          }
          break;
      }
    }

    processing_type |= static_cast<unsigned>(
        field->is_repeated() ?  internal::kRepeatedMask : 0);
    processing_type |= static_cast<unsigned>(
        field->containing_oneof() ? internal::kOneofMask : 0);

    if (field->is_map()) {
      processing_type = internal::TYPE_MAP;
    }

    const unsigned char tag_size =
      WireFormat::TagSize(field->number(), field->type());

    std::map<string, string> vars;
    vars["classname"] = QualifiedClassName(descriptor_);
    if (field->containing_oneof() != NULL) {
      vars["name"] = field->containing_oneof()->name();
      vars["presence"] = SimpleItoa(field->containing_oneof()->index());
    } else {
      vars["name"] = FieldName(field);
      vars["presence"] = SimpleItoa(has_bit_indices_[field->index()]);
    }
    vars["nwtype"] = SimpleItoa(normal_wiretype);
    vars["pwtype"] = SimpleItoa(packed_wiretype);
    vars["ptype"] = SimpleItoa(processing_type);
    vars["tag_size"] = SimpleItoa(tag_size);

    printer->Print(vars,
      "{\n"
      "  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(\n"
      "    $classname$, $name$_),\n"
      "  static_cast<::google::protobuf::uint32>($presence$),\n"
      "  $nwtype$, $pwtype$, $ptype$, $tag_size$\n"
      "},\n");
  }

  return last_field_number;
}

size_t MessageGenerator::GenerateParseAuxTable(io::Printer* printer) {
  if (!table_driven_) {
    return 0;
  }

  std::vector<const FieldDescriptor*> ordered_fields =
      SortFieldsByNumber(descriptor_);

  printer->Print("::google::protobuf::internal::AuxillaryParseTableField(),\n");
  int last_field_number = 1;
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = ordered_fields[i];

    GOOGLE_CHECK_GE(field->number(), last_field_number);
    for (; last_field_number < field->number(); last_field_number++) {
      printer->Print("::google::protobuf::internal::AuxillaryParseTableField(),\n");
    }

    std::map<string, string> vars;
    SetCommonFieldVariables(field, &vars, options_);

    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_ENUM:
        vars["type"] = ClassName(field->enum_type(), true);
        printer->Print(
            vars,
            "{::google::protobuf::internal::AuxillaryParseTableField::enum_aux{"
            "$type$_IsValid}},\n");
        last_field_number++;
        break;
      case FieldDescriptor::CPPTYPE_MESSAGE: {
        if (field->is_map()) {
          vars["classname"] = QualifiedClassName(field->message_type());
          printer->Print(vars,
                         "{::google::protobuf::internal::AuxillaryParseTableField::map_"
                         "aux{&::google::protobuf::internal::ParseMap<$classname$>}},\n");
          last_field_number++;
          break;
        } else {
          vars["classname"] = ClassName(field->message_type(), false);
        }
        vars["ns"] = Namespace(field->message_type());
        vars["type"] = FieldMessageTypeName(field);
        vars["file_namespace"] =
            FileLevelNamespace(field->message_type());

        printer->Print(
            vars,
            "{::google::protobuf::internal::AuxillaryParseTableField::message_aux{\n"
            "  &$ns$::_$classname$_default_instance_}},\n");
        last_field_number++;
        break;
      }
      case FieldDescriptor::CPPTYPE_STRING:
        switch (EffectiveStringCType(field)) {
          case FieldOptions::STRING:
            vars["default"] =
                field->default_value_string().empty()
                    ? "&::google::protobuf::internal::fixed_address_empty_string"
                    : "&" + Namespace(field) + " ::" + classname_ +
                          "::" + MakeDefaultName(field);
            break;
          case FieldOptions::CORD:
          case FieldOptions::STRING_PIECE:
            vars["default"] =
                "\"" + CEscape(field->default_value_string()) + "\"";
            break;
        }
        vars["full_name"] = field->full_name();
        printer->Print(vars,
            "{::google::protobuf::internal::AuxillaryParseTableField::string_aux{\n"
            "  $default$,\n"
            "  \"$full_name$\"\n"
            "}},\n");
        last_field_number++;
        break;
      default:
        break;
    }
  }

  return last_field_number;
}

std::pair<size_t, size_t> MessageGenerator::GenerateOffsets(
    io::Printer* printer) {
  std::map<string, string> variables;
  string full_classname = QualifiedClassName(descriptor_);
  variables["classname"] = full_classname;

  if (HasFieldPresence(descriptor_->file()) || IsMapEntryMessage(descriptor_)) {
    printer->Print(
        variables,
        "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, "
        "_has_bits_),\n");
  } else {
    printer->Print("~0u,  // no _has_bits_\n");
  }
  printer->Print(variables,
                 "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, "
                 "_internal_metadata_),\n");
  if (descriptor_->extension_range_count() > 0) {
    printer->Print(
        variables,
        "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, "
        "_extensions_),\n");
  } else {
    printer->Print("~0u,  // no _extensions_\n");
  }
  if (descriptor_->oneof_decl_count() > 0) {
    printer->Print(variables,
                   "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET("
                   "$classname$, _oneof_case_[0]),\n");
  } else {
    printer->Print("~0u,  // no _oneof_case_\n");
  }
  if (num_weak_fields_ > 0) {
    printer->Print(variables,
                   "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$,"
                   " _weak_field_map_),\n");
  } else {
    printer->Print("~0u,  // no _weak_field_map_\n");
  }
  const int kNumGenericOffsets = 5;  // the number of fixed offsets above
  const size_t offsets = kNumGenericOffsets +
                         descriptor_->field_count() +
                         descriptor_->oneof_decl_count();
  size_t entries = offsets;
  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (field->containing_oneof() || field->options().weak()) {
      printer->Print("offsetof($classname$DefaultTypeInternal, $name$_)",
                     "classname", full_classname, "name", FieldName(field));
    } else {
      printer->Print(
          "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, $name$_)",
          "classname", full_classname, "name", FieldName(field));
    }

    uint32 tag = field_generators_.get(field).CalculateFieldTag();
    if (tag != 0) {
      printer->Print(" | $tag$", "tag", SimpleItoa(tag));
    }

    printer->Print(",\n");
  }

  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    const OneofDescriptor* oneof = descriptor_->oneof_decl(i);
    printer->Print(
        "GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET($classname$, $name$_),\n",
        "classname", full_classname, "name", oneof->name());
  }

  if (IsMapEntryMessage(descriptor_)) {
    entries += 2;
    printer->Print(
        "0,\n"
        "1,\n");
  } else if (HasFieldPresence(descriptor_->file())) {
    entries += has_bit_indices_.size();
    for (int i = 0; i < has_bit_indices_.size(); i++) {
      const string index = has_bit_indices_[i] >= 0 ?
        SimpleItoa(has_bit_indices_[i]) : "~0u";
      printer->Print("$index$,\n", "index", index);
    }
  }

  return std::make_pair(entries, offsets);
}

void MessageGenerator::
GenerateSharedConstructorCode(io::Printer* printer) {
  printer->Print(
    "void $classname$::SharedCtor() {\n",
    "classname", classname_);
  printer->Indent();

  std::vector<bool> processed(optimized_order_.size(), false);
  GenerateConstructorBody(printer, processed, false);

  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "clear_has_$oneof_name$();\n",
        "oneof_name", descriptor_->oneof_decl(i)->name());
  }

  printer->Outdent();
  printer->Print("}\n\n");
}

void MessageGenerator::
GenerateSharedDestructorCode(io::Printer* printer) {
  printer->Print(
    "void $classname$::SharedDtor() {\n",
    "classname", classname_);
  printer->Indent();
  if (SupportsArenas(descriptor_)) {
    printer->Print(
      "GOOGLE_DCHECK(GetArenaNoVirtual() == NULL);\n");
  }
  // Write the destructors for each field except oneof members.
  // optimized_order_ does not contain oneof fields.
  for (int i = 0; i < optimized_order_.size(); i++) {
    const FieldDescriptor* field = optimized_order_[i];
    field_generators_.get(field).GenerateDestructorCode(printer);
  }

  // Generate code to destruct oneofs. Clearing should do the work.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "if (has_$oneof_name$()) {\n"
        "  clear_$oneof_name$();\n"
        "}\n",
        "oneof_name", descriptor_->oneof_decl(i)->name());
  }

  if (num_weak_fields_) {
    printer->Print("_weak_field_map_.ClearAll();\n");
  }
  printer->Outdent();
  printer->Print(
    "}\n"
    "\n");
}

void MessageGenerator::
GenerateArenaDestructorCode(io::Printer* printer) {
  // Generate the ArenaDtor() method. Track whether any fields actually produced
  // code that needs to be called.
  printer->Print(
      "void $classname$::ArenaDtor(void* object) {\n",
      "classname", classname_);
  printer->Indent();

  // This code is placed inside a static method, rather than an ordinary one,
  // since that simplifies Arena's destructor list (ordinary function pointers
  // rather than member function pointers). _this is the object being
  // destructed.
  printer->Print(
      "$classname$* _this = reinterpret_cast< $classname$* >(object);\n"
      // avoid an "unused variable" warning in case no fields have dtor code.
      "(void)_this;\n",
      "classname", classname_);

  bool need_registration = false;
  // Process non-oneof fields first.
  for (int i = 0; i < optimized_order_.size(); i++) {
    const FieldDescriptor* field = optimized_order_[i];
    if (field_generators_.get(field)
                         .GenerateArenaDestructorCode(printer)) {
      need_registration = true;
    }
  }

  // Process oneof fields.
  //
  // Note:  As of 10/5/2016, GenerateArenaDestructorCode does not emit anything
  // and returns false for oneof fields.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    const OneofDescriptor* oneof = descriptor_->oneof_decl(i);

    for (int j = 0; j < oneof->field_count(); j++) {
      const FieldDescriptor* field = oneof->field(j);
      if (field_generators_.get(field)
                           .GenerateArenaDestructorCode(printer)) {
        need_registration = true;
      }
    }
  }
  if (num_weak_fields_) {
    // _this is the object being destructed (we are inside a static method
    // here).
    printer->Print("_this->_weak_field_map_.ClearAll();\n");
    need_registration = true;
  }

  printer->Outdent();
  printer->Print(
      "}\n");

  if (need_registration) {
    printer->Print(
        "inline void $classname$::RegisterArenaDtor(::google::protobuf::Arena* arena) {\n"
        "  if (arena != NULL) {\n"
        "    arena->OwnCustomDestructor(this, &$classname$::ArenaDtor);\n"
        "  }\n"
        "}\n",
        "classname", classname_);
  } else {
    printer->Print(
        "void $classname$::RegisterArenaDtor(::google::protobuf::Arena* arena) {\n"
        "}\n",
        "classname", classname_);
  }
}

void MessageGenerator::GenerateConstructorBody(io::Printer* printer,
                                               std::vector<bool> processed,
                                               bool copy_constructor) const {
  const FieldDescriptor* last_start = NULL;
  // RunMap maps from fields that start each run to the number of fields in that
  // run.  This is optimized for the common case that there are very few runs in
  // a message and that most of the eligible fields appear together.
  typedef hash_map<const FieldDescriptor*, size_t> RunMap;
  RunMap runs;

  for (int i = 0; i < optimized_order_.size(); ++i) {
    const FieldDescriptor* field = optimized_order_[i];
    if ((copy_constructor && IsPOD(field)) ||
        (!copy_constructor && CanConstructByZeroing(field, options_))) {
      if (last_start == NULL) {
        last_start = field;
      }

      runs[last_start]++;
    } else {
      last_start = NULL;
    }
  }

  string pod_template;
  if (copy_constructor) {
    pod_template =
        "::memcpy(&$first$_, &from.$first$_,\n"
        "  static_cast<size_t>(reinterpret_cast<char*>(&$last$_) -\n"
        "  reinterpret_cast<char*>(&$first$_)) + sizeof($last$_));\n";
  } else {
    pod_template =
        "::memset(&$first$_, 0, static_cast<size_t>(\n"
        "    reinterpret_cast<char*>(&$last$_) -\n"
        "    reinterpret_cast<char*>(&$first$_)) + sizeof($last$_));\n";
  }

  for (int i = 0; i < optimized_order_.size(); ++i) {
    if (processed[i]) {
      continue;
    }

    const FieldDescriptor* field = optimized_order_[i];
    RunMap::const_iterator it = runs.find(field);

    // We only apply the memset technique to runs of more than one field, as
    // assignment is better than memset for generated code clarity.
    if (it != runs.end() && it->second > 1) {
      // Use a memset, then skip run_length fields.
      const size_t run_length = it->second;
      const string first_field_name = FieldName(field);
      const string last_field_name =
          FieldName(optimized_order_[i + run_length - 1]);

      printer->Print(pod_template.c_str(),
        "first", first_field_name,
        "last", last_field_name);

      i += run_length - 1;
      // ++i at the top of the loop.
    } else {
      if (copy_constructor) {
        field_generators_.get(field).GenerateCopyConstructorCode(printer);
      } else {
        field_generators_.get(field).GenerateConstructorCode(printer);
      }
    }
  }
}

void MessageGenerator::
GenerateStructors(io::Printer* printer) {
  string superclass;
  superclass = SuperClassName(descriptor_, options_);
  string initializer_with_arena = superclass + "()";

  if (descriptor_->extension_range_count() > 0) {
    initializer_with_arena += ",\n  _extensions_(arena)";
  }

  initializer_with_arena += ",\n  _internal_metadata_(arena)";

  // Initialize member variables with arena constructor.
  for (int i = 0; i < optimized_order_.size(); i++) {
    const FieldDescriptor* field = optimized_order_[i];

    bool has_arena_constructor = field->is_repeated();
    if (has_arena_constructor) {
      initializer_with_arena += string(",\n  ") +
          FieldName(field) + string("_(arena)");
    }
  }

  if (IsAnyMessage(descriptor_)) {
    initializer_with_arena += ",\n  _any_metadata_(&type_url_, &value_)";
  }
  if (num_weak_fields_ > 0) {
    initializer_with_arena += ", _weak_field_map_(arena)";
  }

  string initializer_null = superclass + "(), _internal_metadata_(NULL)";
  if (IsAnyMessage(descriptor_)) {
    initializer_null += ", _any_metadata_(&type_url_, &value_)";
  }
  if (num_weak_fields_ > 0) {
    initializer_null += ", _weak_field_map_(nullptr)";
  }

  printer->Print(
      "$classname$::$classname$()\n"
      "  : $initializer$ {\n"
      "  ::google::protobuf::internal::InitSCC(\n"
      "      &$file_namespace$::scc_info_$scc_name$.base);\n"
      "  SharedCtor();\n"
      "  // @@protoc_insertion_point(constructor:$full_name$)\n"
      "}\n",
      "classname", classname_, "full_name", descriptor_->full_name(),
      "scc_name", scc_name_, "initializer", initializer_null, "file_namespace",
      FileLevelNamespace(descriptor_));

  if (SupportsArenas(descriptor_)) {
    printer->Print(
        "$classname$::$classname$(::google::protobuf::Arena* arena)\n"
        "  : $initializer$ {\n"
        "  "
        "::google::protobuf::internal::InitSCC(&$file_namespace$::scc_info_$scc_name$."
        "base);\n"
        "  SharedCtor();\n"
        "  RegisterArenaDtor(arena);\n"
        "  // @@protoc_insertion_point(arena_constructor:$full_name$)\n"
        "}\n",
        "initializer", initializer_with_arena, "classname", classname_,
        "superclass", superclass, "full_name", descriptor_->full_name(),
        "scc_name", scc_name_, "file_namespace",
        FileLevelNamespace(descriptor_));
  }

  // Generate the copy constructor.
  if (UsingImplicitWeakFields(descriptor_->file(), options_)) {
    // If we are in lite mode and using implicit weak fields, we generate a
    // one-liner copy constructor that delegates to MergeFrom. This saves some
    // code size and also cuts down on the complexity of implicit weak fields.
    // We might eventually want to do this for all lite protos.
    printer->Print(
      "$classname$::$classname$(const $classname$& from)\n"
      "  : $classname$() {\n"
      "  MergeFrom(from);\n"
      "}\n",
      "classname", classname_);
  } else {
    printer->Print(
      "$classname$::$classname$(const $classname$& from)\n"
      "  : $superclass$()",
      "classname", classname_,
      "superclass", superclass,
      "full_name", descriptor_->full_name());
    printer->Indent();
    printer->Indent();
    printer->Indent();

    printer->Print(
        ",\n_internal_metadata_(NULL)");

    if (HasFieldPresence(descriptor_->file())) {
        printer->Print(",\n_has_bits_(from._has_bits_)");
    }

    std::vector<bool> processed(optimized_order_.size(), false);
    for (int i = 0; i < optimized_order_.size(); ++i) {
      const FieldDescriptor* field = optimized_order_[i];

      if (!(field->is_repeated() && !(field->is_map()))
          ) {
        continue;
      }

      processed[i] = true;
      printer->Print(",\n$name$_(from.$name$_)",
                     "name", FieldName(field));
    }

    if (IsAnyMessage(descriptor_)) {
      printer->Print(",\n_any_metadata_(&type_url_, &value_)");
    }
    if (num_weak_fields_ > 0) {
      printer->Print(",\n_weak_field_map_(from._weak_field_map_)");
    }

    printer->Outdent();
    printer->Outdent();
    printer->Print(" {\n");

    printer->Print(
        "_internal_metadata_.MergeFrom(from._internal_metadata_);\n");

    if (descriptor_->extension_range_count() > 0) {
      printer->Print("_extensions_.MergeFrom(from._extensions_);\n");
    }

    GenerateConstructorBody(printer, processed, true);

    // Copy oneof fields. Oneof field requires oneof case check.
    for (int i = 0; i < descriptor_->oneof_decl_count(); ++i) {
      printer->Print(
          "clear_has_$oneofname$();\n"
          "switch (from.$oneofname$_case()) {\n",
          "oneofname", descriptor_->oneof_decl(i)->name());
      printer->Indent();
      for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
        const FieldDescriptor* field = descriptor_->oneof_decl(i)->field(j);
        printer->Print(
            "case k$field_name$: {\n",
            "field_name", UnderscoresToCamelCase(field->name(), true));
        printer->Indent();
        field_generators_.get(field).GenerateMergingCode(printer);
        printer->Print(
            "break;\n");
        printer->Outdent();
        printer->Print(
            "}\n");
      }
      printer->Print(
          "case $cap_oneof_name$_NOT_SET: {\n"
          "  break;\n"
          "}\n",
          "oneof_index",
          SimpleItoa(descriptor_->oneof_decl(i)->index()),
          "cap_oneof_name",
          ToUpper(descriptor_->oneof_decl(i)->name()));
      printer->Outdent();
      printer->Print(
          "}\n");
    }

    printer->Outdent();
    printer->Print(
      "  // @@protoc_insertion_point(copy_constructor:$full_name$)\n"
      "}\n"
      "\n",
      "full_name", descriptor_->full_name());
  }

  // Generate the shared constructor code.
  GenerateSharedConstructorCode(printer);

  // Generate the destructor.
  printer->Print(
    "$classname$::~$classname$() {\n"
    "  // @@protoc_insertion_point(destructor:$full_name$)\n"
    "  SharedDtor();\n"
    "}\n"
    "\n",
    "classname", classname_,
    "full_name", descriptor_->full_name());

  // Generate the shared destructor code.
  GenerateSharedDestructorCode(printer);

  // Generate the arena-specific destructor code.
  if (SupportsArenas(descriptor_)) {
    GenerateArenaDestructorCode(printer);
  }

  // Generate SetCachedSize.
  printer->Print(
      "void $classname$::SetCachedSize(int size) const {\n"
      "  _cached_size_.Set(size);\n"
      "}\n",
      "classname", classname_);

  // Only generate this member if it's not disabled.
  if (HasDescriptorMethods(descriptor_->file(), options_) &&
      !descriptor_->options().no_standard_descriptor_accessor()) {
    printer->Print(
        "const ::google::protobuf::Descriptor* $classname$::descriptor() {\n"
        "  ::$file_namespace$::protobuf_AssignDescriptorsOnce();\n"
        "  return ::"
        "$file_namespace$::file_level_metadata[kIndexInFileMessages]."
        "descriptor;\n"
        "}\n"
        "\n",
        "classname", classname_, "file_namespace",
        FileLevelNamespace(descriptor_));
  }

  printer->Print(
      "const $classname$& $classname$::default_instance() {\n"
      "  "
      "::google::protobuf::internal::InitSCC(&$file_namespace$::scc_info_$scc_name$.base)"
      ";\n"
      "  return *internal_default_instance();\n"
      "}\n\n",
      "classname", classname_, "scc_name", scc_name_, "file_namespace",
      FileLevelNamespace(descriptor_));
}

void MessageGenerator::GenerateSourceInProto2Namespace(io::Printer* printer) {
  printer->Print(
      "template<> "
      "GOOGLE_PROTOBUF_ATTRIBUTE_NOINLINE "
      "$classname$* Arena::CreateMaybeMessage< $classname$ >(Arena* arena) {\n"
      "  return Arena::$create_func$Internal< $classname$ >(arena);\n"
      "}\n",
      "classname", QualifiedClassName(descriptor_),
      "create_func", MessageCreateFunction(descriptor_));
}

// Return the number of bits set in n, a non-negative integer.
static int popcnt(uint32 n) {
  int result = 0;
  while (n != 0) {
    result += (n & 1);
    n = n / 2;
  }
  return result;
}

bool MessageGenerator::MaybeGenerateOptionalFieldCondition(
    io::Printer* printer, const FieldDescriptor* field,
    int expected_has_bits_index) {
  int has_bit_index = has_bit_indices_[field->index()];
  if (!field->options().weak() &&
      expected_has_bits_index == has_bit_index / 32) {
    const string mask =
        StrCat(strings::Hex(1u << (has_bit_index % 32), strings::ZERO_PAD_8));
    printer->Print(
        "if (cached_has_bits & 0x$mask$u) {\n",
        "mask", mask);
    return true;
  }
  return false;
}

void MessageGenerator::
GenerateClear(io::Printer* printer) {
  // Performance tuning parameters
  const int kMaxUnconditionalPrimitiveBytesClear = 4;

  printer->Print(
      "void $classname$::Clear() {\n"
      "// @@protoc_insertion_point(message_clear_start:$full_name$)\n",
      "classname", classname_, "full_name", descriptor_->full_name());
  printer->Indent();

  printer->Print(
      // TODO(jwb): It would be better to avoid emitting this if it is not used,
      // rather than emitting a workaround for the resulting warning.
      "::google::protobuf::uint32 cached_has_bits = 0;\n"
      "// Prevent compiler warnings about cached_has_bits being unused\n"
      "(void) cached_has_bits;\n\n");

  int cached_has_bit_index = -1;

  // Step 1: Extensions
  if (descriptor_->extension_range_count() > 0) {
    printer->Print("_extensions_.Clear();\n");
  }

  int unconditional_budget = kMaxUnconditionalPrimitiveBytesClear;
  for (int i = 0; i < optimized_order_.size(); i++) {
    const FieldDescriptor* field = optimized_order_[i];

    if (!CanInitializeByZeroing(field)) {
      continue;
    }

    unconditional_budget -= EstimateAlignmentSize(field);
  }

  std::vector<std::vector<const FieldDescriptor*> > chunks_frag = CollectFields(
      optimized_order_,
      MatchRepeatedAndHasByteAndZeroInits(
          &has_bit_indices_, HasFieldPresence(descriptor_->file())));

  // Merge next non-zero initializable chunk if it has the same has_byte index
  // and not meeting unconditional clear condition.
  std::vector<std::vector<const FieldDescriptor*> > chunks;
  if (!HasFieldPresence(descriptor_->file())) {
    // Don't bother with merging without has_bit field.
    chunks = chunks_frag;
  } else {
    // Note that only the next chunk is considered for merging.
    for (int i = 0; i < chunks_frag.size(); i++) {
      chunks.push_back(chunks_frag[i]);
      const FieldDescriptor* field = chunks_frag[i].front();
      const FieldDescriptor* next_field =
          (i + 1) < chunks_frag.size() ? chunks_frag[i + 1].front() : nullptr;
      if (CanInitializeByZeroing(field) &&
          (chunks_frag[i].size() == 1 || unconditional_budget < 0) &&
          next_field != nullptr &&
          has_bit_indices_[field->index()] / 8 ==
              has_bit_indices_[next_field->index()] / 8) {
        GOOGLE_CHECK(!CanInitializeByZeroing(next_field));
        // Insert next chunk to the current one and skip next chunk.
        chunks.back().insert(chunks.back().end(), chunks_frag[i + 1].begin(),
                             chunks_frag[i + 1].end());
        i++;
      }
    }
  }

  for (int chunk_index = 0; chunk_index < chunks.size(); chunk_index++) {
    std::vector<const FieldDescriptor*>& chunk = chunks[chunk_index];
    GOOGLE_CHECK(!chunk.empty());

    // Step 2: Repeated fields don't use _has_bits_; emit code to clear them
    // here.
    if (chunk.front()->is_repeated()) {
      for (int i = 0; i < chunk.size(); i++) {
        const FieldDescriptor* field = chunk[i];
        const FieldGenerator& generator = field_generators_.get(field);

        generator.GenerateMessageClearingCode(printer);
      }
      continue;
    }

    // Step 3: Non-repeated fields that can be cleared by memset-to-0, then
    // non-repeated, non-zero initializable fields.
    int last_chunk = HasFieldPresence(descriptor_->file())
                         ? has_bit_indices_[chunk.front()->index()] / 8
                         : 0;
    int last_chunk_start = 0;
    int memset_run_start = -1;
    int memset_run_end = -1;

    for (int i = 0; i < chunk.size(); i++) {
      const FieldDescriptor* field = chunk[i];
      if (CanInitializeByZeroing(field)) {
        if (memset_run_start == -1) {
          memset_run_start = i;
        }
        memset_run_end = i;
      }
    }

    const bool have_outer_if =
        HasFieldPresence(descriptor_->file()) && chunk.size() > 1 &&
        (memset_run_end != chunk.size() - 1 || unconditional_budget < 0);

    if (have_outer_if) {
      uint32 last_chunk_mask = GenChunkMask(chunk, has_bit_indices_);
      const int count = popcnt(last_chunk_mask);

      // Check (up to) 8 has_bits at a time if we have more than one field in
      // this chunk.  Due to field layout ordering, we may check
      // _has_bits_[last_chunk * 8 / 32] multiple times.
      GOOGLE_DCHECK_LE(2, count);
      GOOGLE_DCHECK_GE(8, count);

      if (cached_has_bit_index != last_chunk / 4) {
        cached_has_bit_index = last_chunk / 4;
        printer->Print("cached_has_bits = _has_bits_[$idx$];\n", "idx",
                       SimpleItoa(cached_has_bit_index));
      }
      printer->Print("if (cached_has_bits & $mask$u) {\n", "mask",
                     SimpleItoa(last_chunk_mask));
      printer->Indent();
    }

    if (memset_run_start != -1) {
      if (memset_run_start == memset_run_end) {
        // For clarity, do not memset a single field.
        const FieldGenerator& generator =
            field_generators_.get(chunk[memset_run_start]);
        generator.GenerateMessageClearingCode(printer);
      } else {
        const string first_field_name = FieldName(chunk[memset_run_start]);
        const string last_field_name = FieldName(chunk[memset_run_end]);

        printer->Print(
            "::memset(&$first$_, 0, static_cast<size_t>(\n"
            "    reinterpret_cast<char*>(&$last$_) -\n"
            "    reinterpret_cast<char*>(&$first$_)) + sizeof($last$_));\n",
            "first", first_field_name, "last", last_field_name);
      }

      // Advance last_chunk_start to skip over the fields we zeroed/memset.
      last_chunk_start = memset_run_end + 1;
    }

    // Go back and emit clears for each of the fields we processed.
    for (int j = last_chunk_start; j < chunk.size(); j++) {
      const FieldDescriptor* field = chunk[j];
      const string fieldname = FieldName(field);
      const FieldGenerator& generator = field_generators_.get(field);

      // It's faster to just overwrite primitive types, but we should only
      // clear strings and messages if they were set.
      //
      // TODO(kenton):  Let the CppFieldGenerator decide this somehow.
      bool should_check_bit =
          field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE ||
          field->cpp_type() == FieldDescriptor::CPPTYPE_STRING;

      bool have_enclosing_if = false;
      if (should_check_bit &&
          // If no field presence, then always clear strings/messages as well.
          HasFieldPresence(descriptor_->file())) {
        if (!field->options().weak() &&
            cached_has_bit_index != (has_bit_indices_[field->index()] / 32)) {
          cached_has_bit_index = (has_bit_indices_[field->index()] / 32);
          printer->Print("cached_has_bits = _has_bits_[$new_index$];\n",
                         "new_index", SimpleItoa(cached_has_bit_index));
        }
        if (!MaybeGenerateOptionalFieldCondition(printer, field,
                                                 cached_has_bit_index)) {
          printer->Print("if (has_$name$()) {\n", "name", fieldname);
        }
        printer->Indent();
        have_enclosing_if = true;
      }

      generator.GenerateMessageClearingCode(printer);

      if (have_enclosing_if) {
        printer->Outdent();
        printer->Print("}\n");
      }
    }

    if (have_outer_if) {
      printer->Outdent();
      printer->Print("}\n");
    }
  }

  // Step 4: Unions.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "clear_$oneof_name$();\n",
        "oneof_name", descriptor_->oneof_decl(i)->name());
  }

  if (num_weak_fields_) {
    printer->Print("_weak_field_map_.ClearAll();\n");
  }

  if (HasFieldPresence(descriptor_->file())) {
    // Step 5: Everything else.
    printer->Print("_has_bits_.Clear();\n");
  }

  printer->Print("_internal_metadata_.Clear();\n");

  printer->Outdent();
  printer->Print("}\n");
}

void MessageGenerator::
GenerateOneofClear(io::Printer* printer) {
  // Generated function clears the active field and union case (e.g. foo_case_).
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    std::map<string, string> oneof_vars;
    oneof_vars["classname"] = classname_;
    oneof_vars["oneofname"] = descriptor_->oneof_decl(i)->name();
    oneof_vars["full_name"] = descriptor_->full_name();
    string message_class;

    printer->Print(oneof_vars,
                   "void $classname$::clear_$oneofname$() {\n"
                   "// @@protoc_insertion_point(one_of_clear_start:"
                   "$full_name$)\n");
    printer->Indent();
    printer->Print(oneof_vars,
        "switch ($oneofname$_case()) {\n");
    printer->Indent();
    for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
      const FieldDescriptor* field = descriptor_->oneof_decl(i)->field(j);
      printer->Print(
          "case k$field_name$: {\n",
          "field_name", UnderscoresToCamelCase(field->name(), true));
      printer->Indent();
      // We clear only allocated objects in oneofs
      if (!IsStringOrMessage(field)) {
        printer->Print(
            "// No need to clear\n");
      } else {
        field_generators_.get(field).GenerateClearingCode(printer);
      }
      printer->Print(
          "break;\n");
      printer->Outdent();
      printer->Print(
          "}\n");
    }
    printer->Print(
        "case $cap_oneof_name$_NOT_SET: {\n"
        "  break;\n"
        "}\n",
        "cap_oneof_name",
        ToUpper(descriptor_->oneof_decl(i)->name()));
    printer->Outdent();
    printer->Print(
        "}\n"
        "_oneof_case_[$oneof_index$] = $cap_oneof_name$_NOT_SET;\n",
        "oneof_index", SimpleItoa(i),
        "cap_oneof_name",
        ToUpper(descriptor_->oneof_decl(i)->name()));
    printer->Outdent();
    printer->Print(
        "}\n"
        "\n");
  }
}

void MessageGenerator::
GenerateSwap(io::Printer* printer) {
  if (SupportsArenas(descriptor_)) {
    // Generate the Swap member function. This is a lightweight wrapper around
    // UnsafeArenaSwap() / MergeFrom() with temporaries, depending on the memory
    // ownership situation: swapping across arenas or between an arena and a
    // heap requires copying.
    printer->Print(
        "void $classname$::Swap($classname$* other) {\n"
        "  if (other == this) return;\n"
        "  if (GetArenaNoVirtual() == other->GetArenaNoVirtual()) {\n"
        "    InternalSwap(other);\n"
        "  } else {\n"
        "    $classname$* temp = New(GetArenaNoVirtual());\n"
        "    temp->MergeFrom(*other);\n"
        "    other->CopyFrom(*this);\n"
        "    InternalSwap(temp);\n"
        "    if (GetArenaNoVirtual() == NULL) {\n"
        "      delete temp;\n"
        "    }\n"
        "  }\n"
        "}\n"
        "void $classname$::UnsafeArenaSwap($classname$* other) {\n"
        "  if (other == this) return;\n"
        "  GOOGLE_DCHECK(GetArenaNoVirtual() == other->GetArenaNoVirtual());\n"
        "  InternalSwap(other);\n"
        "}\n",
        "classname", classname_);
  } else {
    printer->Print(
        "void $classname$::Swap($classname$* other) {\n"
        "  if (other == this) return;\n"
        "  InternalSwap(other);\n"
        "}\n",
        "classname", classname_);
  }

  // Generate the UnsafeArenaSwap member function.
  printer->Print("void $classname$::InternalSwap($classname$* other) {\n",
                 "classname", classname_);
  printer->Indent();
  printer->Print("using std::swap;\n");

  if (HasGeneratedMethods(descriptor_->file(), options_)) {
    for (int i = 0; i < optimized_order_.size(); i++) {
      // optimized_order_ does not contain oneof fields, but the field
      // generators for these fields do not emit swapping code on their own.
      const FieldDescriptor* field = optimized_order_[i];
      field_generators_.get(field).GenerateSwappingCode(printer);
    }

    for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
      printer->Print(
        "swap($oneof_name$_, other->$oneof_name$_);\n"
        "swap(_oneof_case_[$i$], other->_oneof_case_[$i$]);\n",
        "oneof_name", descriptor_->oneof_decl(i)->name(),
        "i", SimpleItoa(i));
    }

    if (HasFieldPresence(descriptor_->file())) {
      for (int i = 0; i < HasBitsSize() / 4; ++i) {
        printer->Print("swap(_has_bits_[$i$], other->_has_bits_[$i$]);\n",
                       "i", SimpleItoa(i));
      }
    }

    printer->Print("_internal_metadata_.Swap(&other->_internal_metadata_);\n");

    if (descriptor_->extension_range_count() > 0) {
      printer->Print("_extensions_.Swap(&other->_extensions_);\n");
    }
    if (num_weak_fields_) {
      printer->Print(
          "_weak_field_map_.UnsafeArenaSwap(&other->_weak_field_map_);\n");
    }
  } else {
    printer->Print("GetReflection()->Swap(this, other);");
  }

  printer->Outdent();
  printer->Print("}\n");
}

void MessageGenerator::
GenerateMergeFrom(io::Printer* printer) {
  if (HasDescriptorMethods(descriptor_->file(), options_)) {
    // Generate the generalized MergeFrom (aka that which takes in the Message
    // base class as a parameter).
    printer->Print(
        "void $classname$::MergeFrom(const ::google::protobuf::Message& from) {\n"
        "// @@protoc_insertion_point(generalized_merge_from_start:"
        "$full_name$)\n"
        "  GOOGLE_DCHECK_NE(&from, this);\n",
        "classname", classname_, "full_name", descriptor_->full_name());
    printer->Indent();

    // Cast the message to the proper type. If we find that the message is
    // *not* of the proper type, we can still call Merge via the reflection
    // system, as the GOOGLE_CHECK above ensured that we have the same descriptor
    // for each message.
    printer->Print(
      "const $classname$* source =\n"
      "    ::google::protobuf::internal::DynamicCastToGenerated<const $classname$>(\n"
      "        &from);\n"
      "if (source == NULL) {\n"
      "// @@protoc_insertion_point(generalized_merge_from_cast_fail:"
      "$full_name$)\n"
      "  ::google::protobuf::internal::ReflectionOps::Merge(from, this);\n"
      "} else {\n"
      "// @@protoc_insertion_point(generalized_merge_from_cast_success:"
      "$full_name$)\n"
      "  MergeFrom(*source);\n"
      "}\n",
      "classname", classname_, "full_name", descriptor_->full_name());

    printer->Outdent();
    printer->Print("}\n\n");
  } else {
    // Generate CheckTypeAndMergeFrom().
    printer->Print(
      "void $classname$::CheckTypeAndMergeFrom(\n"
      "    const ::google::protobuf::MessageLite& from) {\n"
      "  MergeFrom(*::google::protobuf::down_cast<const $classname$*>(&from));\n"
      "}\n"
      "\n",
      "classname", classname_);
  }

  // Generate the class-specific MergeFrom, which avoids the GOOGLE_CHECK and cast.
  printer->Print(
      "void $classname$::MergeFrom(const $classname$& from) {\n"
      "// @@protoc_insertion_point(class_specific_merge_from_start:"
      "$full_name$)\n"
      "  GOOGLE_DCHECK_NE(&from, this);\n",
      "classname", classname_, "full_name", descriptor_->full_name());
  printer->Indent();

  if (descriptor_->extension_range_count() > 0) {
    printer->Print("_extensions_.MergeFrom(from._extensions_);\n");
  }

  printer->Print(
    "_internal_metadata_.MergeFrom(from._internal_metadata_);\n"
    "::google::protobuf::uint32 cached_has_bits = 0;\n"
    "(void) cached_has_bits;\n\n");

  // cached_has_bit_index maintains that:
  //   cached_has_bits = from._has_bits_[cached_has_bit_index]
  // for cached_has_bit_index >= 0
  int cached_has_bit_index = -1;

  int last_i = -1;
  for (int i = 0; i < optimized_order_.size(); ) {
    // Detect infinite loops.
    GOOGLE_CHECK_NE(i, last_i);
    last_i = i;

    // Merge Repeated fields. These fields do not require a
    // check as we can simply iterate over them.
    for (; i < optimized_order_.size(); i++) {
      const FieldDescriptor* field = optimized_order_[i];
      if (!field->is_repeated()) {
        break;
      }

      const FieldGenerator& generator = field_generators_.get(field);
      generator.GenerateMergingCode(printer);
    }

    // Merge Optional and Required fields (after a _has_bit_ check).
    int last_chunk = -1;
    int last_chunk_start = -1;
    int last_chunk_end = -1;
    uint32 last_chunk_mask = 0;
    for (; i < optimized_order_.size(); i++) {
      const FieldDescriptor* field = optimized_order_[i];
      if (field->is_repeated()) {
        break;
      }

      // "index" defines where in the _has_bits_ the field appears.
      // "i" is our loop counter within optimized_order_.
      int index = HasFieldPresence(descriptor_->file()) ?
          has_bit_indices_[field->index()] : 0;
      int chunk = index / 8;

      if (last_chunk == -1) {
        last_chunk = chunk;
        last_chunk_start = i;
      } else if (chunk != last_chunk) {
        // Emit the fields for this chunk so far.
        break;
      }

      last_chunk_end = i;
      last_chunk_mask |= static_cast<uint32>(1) << (index % 32);
    }

    if (last_chunk != -1) {
      GOOGLE_DCHECK_NE(-1, last_chunk_start);
      GOOGLE_DCHECK_NE(-1, last_chunk_end);
      GOOGLE_DCHECK_NE(0, last_chunk_mask);

      const int count = popcnt(last_chunk_mask);
      const bool have_outer_if = HasFieldPresence(descriptor_->file()) &&
          (last_chunk_start != last_chunk_end);

      if (have_outer_if) {
        // Check (up to) 8 has_bits at a time if we have more than one field in
        // this chunk.  Due to field layout ordering, we may check
        // _has_bits_[last_chunk * 8 / 32] multiple times.
        GOOGLE_DCHECK_LE(2, count);
        GOOGLE_DCHECK_GE(8, count);

        if (cached_has_bit_index != last_chunk / 4) {
          int new_index = last_chunk / 4;
          printer->Print("cached_has_bits = from._has_bits_[$new_index$];\n",
                         "new_index", SimpleItoa(new_index));
          cached_has_bit_index = new_index;
        }

        printer->Print(
          "if (cached_has_bits & $mask$u) {\n",
          "mask", SimpleItoa(last_chunk_mask));
        printer->Indent();
      }

      // Go back and emit clears for each of the fields we processed.
      bool deferred_has_bit_changes = false;
      for (int j = last_chunk_start; j <= last_chunk_end; j++) {
        const FieldDescriptor* field = optimized_order_[j];
        const FieldGenerator& generator = field_generators_.get(field);

        bool have_enclosing_if = false;
        if (HasFieldPresence(descriptor_->file())) {
          // Attempt to use the state of cached_has_bits, if possible.
          int has_bit_index = has_bit_indices_[field->index()];
          if (!field->options().weak() &&
              cached_has_bit_index == has_bit_index / 32) {
            const string mask = StrCat(
                strings::Hex(1u << (has_bit_index % 32),
                strings::ZERO_PAD_8));

            printer->Print(
                "if (cached_has_bits & 0x$mask$u) {\n", "mask", mask);
          } else {
            printer->Print(
              "if (from.has_$name$()) {\n",
              "name", FieldName(field));
          }

          printer->Indent();
          have_enclosing_if = true;
        } else {
          // Merge semantics without true field presence: primitive fields are
          // merged only if non-zero (numeric) or non-empty (string).
          have_enclosing_if = EmitFieldNonDefaultCondition(
              printer, "from.", field);
        }

        if (have_outer_if && IsPOD(field)) {
          // GenerateCopyConstructorCode for enum and primitive scalar fields
          // does not do _has_bits_ modifications.  We defer _has_bits_
          // manipulation until the end of the outer if.
          //
          // This can reduce the number of loads/stores by up to 7 per 8 fields.
          deferred_has_bit_changes = true;
          generator.GenerateCopyConstructorCode(printer);
        } else {
          generator.GenerateMergingCode(printer);
        }

        if (have_enclosing_if) {
          printer->Outdent();
          printer->Print("}\n");
        }
      }

      if (have_outer_if) {
        if (deferred_has_bit_changes) {
          // Flush the has bits for the primitives we deferred.
          GOOGLE_CHECK_LE(0, cached_has_bit_index);
          printer->Print(
              "_has_bits_[$index$] |= cached_has_bits;\n",
              "index", SimpleItoa(cached_has_bit_index));
        }

        printer->Outdent();
        printer->Print("}\n");
      }
    }
  }

  // Merge oneof fields. Oneof field requires oneof case check.
  for (int i = 0; i < descriptor_->oneof_decl_count(); ++i) {
    printer->Print(
        "switch (from.$oneofname$_case()) {\n",
        "oneofname", descriptor_->oneof_decl(i)->name());
    printer->Indent();
    for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
      const FieldDescriptor* field = descriptor_->oneof_decl(i)->field(j);
      printer->Print(
          "case k$field_name$: {\n",
          "field_name", UnderscoresToCamelCase(field->name(), true));
      printer->Indent();
      field_generators_.get(field).GenerateMergingCode(printer);
      printer->Print(
          "break;\n");
      printer->Outdent();
      printer->Print(
          "}\n");
    }
    printer->Print(
        "case $cap_oneof_name$_NOT_SET: {\n"
        "  break;\n"
        "}\n",
        "cap_oneof_name",
        ToUpper(descriptor_->oneof_decl(i)->name()));
    printer->Outdent();
    printer->Print(
        "}\n");
  }
  if (num_weak_fields_) {
    printer->Print("_weak_field_map_.MergeFrom(from._weak_field_map_);\n");
  }

  printer->Outdent();
  printer->Print("}\n");
}

void MessageGenerator::
GenerateCopyFrom(io::Printer* printer) {
  if (HasDescriptorMethods(descriptor_->file(), options_)) {
    // Generate the generalized CopyFrom (aka that which takes in the Message
    // base class as a parameter).
    printer->Print(
        "void $classname$::CopyFrom(const ::google::protobuf::Message& from) {\n"
        "// @@protoc_insertion_point(generalized_copy_from_start:"
        "$full_name$)\n",
        "classname", classname_, "full_name", descriptor_->full_name());
    printer->Indent();

    printer->Print(
      "if (&from == this) return;\n"
      "Clear();\n"
      "MergeFrom(from);\n");

    printer->Outdent();
    printer->Print("}\n\n");
  }

  // Generate the class-specific CopyFrom.
  printer->Print(
      "void $classname$::CopyFrom(const $classname$& from) {\n"
      "// @@protoc_insertion_point(class_specific_copy_from_start:"
      "$full_name$)\n",
      "classname", classname_, "full_name", descriptor_->full_name());
  printer->Indent();

  printer->Print(
    "if (&from == this) return;\n"
    "Clear();\n"
    "MergeFrom(from);\n");

  printer->Outdent();
  printer->Print("}\n");
}

void MessageGenerator::
GenerateMergeFromCodedStream(io::Printer* printer) {
  std::map<string, string> vars;
  SetUnknkownFieldsVariable(descriptor_, options_, &vars);
  if (descriptor_->options().message_set_wire_format()) {
    // Special-case MessageSet.
    vars["classname"] = classname_;
    printer->Print(vars,
      "bool $classname$::MergePartialFromCodedStream(\n"
      "    ::google::protobuf::io::CodedInputStream* input) {\n"
      "  return _extensions_.ParseMessageSet(input,\n"
      "      internal_default_instance(), $mutable_unknown_fields$);\n"
      "}\n");
    return;
  }
  std::vector<const FieldDescriptor*> ordered_fields =
      SortFieldsByNumber(descriptor_);

  printer->Print(
    "bool $classname$::MergePartialFromCodedStream(\n"
    "    ::google::protobuf::io::CodedInputStream* input) {\n",
    "classname", classname_);

  if (table_driven_) {
    printer->Indent();

    const string lite = UseUnknownFieldSet(descriptor_->file(), options_) ?
        "" : "Lite";

    printer->Print(
        "return ::google::protobuf::internal::MergePartialFromCodedStream$lite$(\n"
        "    this,\n"
        "    ::$file_namespace$::TableStruct::schema[\n"
        "      $classname$::kIndexInFileMessages],\n"
        "    input);\n",
        "classname", classname_, "file_namespace",
        FileLevelNamespace(descriptor_), "lite", lite);

    printer->Outdent();

    printer->Print("}\n");
    return;
  }

  if (SupportsArenas(descriptor_)) {
    for (int i = 0; i < ordered_fields.size(); i++) {
      const FieldDescriptor* field = ordered_fields[i];
      const FieldGenerator& field_generator = field_generators_.get(field);
      if (field_generator.MergeFromCodedStreamNeedsArena()) {
        printer->Print(
          "  ::google::protobuf::Arena* arena = GetArenaNoVirtual();\n");
        break;
      }
    }
  }

  printer->Print(
      "#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto "
      "failure\n"
      "  ::google::protobuf::uint32 tag;\n");

  if (!UseUnknownFieldSet(descriptor_->file(), options_)) {
    printer->Print(
        "  ::google::protobuf::internal::LiteUnknownFieldSetter unknown_fields_setter(\n"
        "      &_internal_metadata_);\n"
        "  ::google::protobuf::io::StringOutputStream unknown_fields_output(\n"
        "      unknown_fields_setter.buffer());\n"
        "  ::google::protobuf::io::CodedOutputStream unknown_fields_stream(\n"
        "      &unknown_fields_output, false);\n",
        "classname", classname_);
  }

  printer->Print(
    "  // @@protoc_insertion_point(parse_start:$full_name$)\n",
    "full_name", descriptor_->full_name());

  printer->Indent();
  printer->Print("for (;;) {\n");
  printer->Indent();

  // To calculate the maximum tag to expect, we look at the highest-numbered
  // field. We need to be prepared to handle more than one wire type if that
  // field is a packable repeated field, so to simplify things we assume the
  // highest possible wire type of 5.
  uint32 maxtag =
      ordered_fields.empty() ? 0 : ordered_fields.back()->number() * 8 + 5;
  const int kCutoff0 = 127;               // fits in 1-byte varint
  const int kCutoff1 = (127 << 7) + 127;  // fits in 2-byte varint

  // We need to capture the last tag when parsing if this is a Group type, as
  // our caller will verify (via CodedInputStream::LastTagWas) that the correct
  // closing tag was received.
  bool capture_last_tag = false;
  const Descriptor* parent = descriptor_->containing_type();
  if (parent) {
    for (int i = 0; i < parent->field_count(); i++) {
      const FieldDescriptor* field = parent->field(i);
      if (field->type() == FieldDescriptor::TYPE_GROUP &&
          field->message_type() == descriptor_) {
        capture_last_tag = true;
        break;
      }
    }

    for (int i = 0; i < parent->extension_count(); i++) {
      const FieldDescriptor* field = parent->extension(i);
      if (field->type() == FieldDescriptor::TYPE_GROUP &&
          field->message_type() == descriptor_) {
        capture_last_tag = true;
        break;
      }
    }
  }

  for (int i = 0; i < descriptor_->file()->extension_count(); i++) {
    const FieldDescriptor* field = descriptor_->file()->extension(i);
    if (field->type() == FieldDescriptor::TYPE_GROUP &&
        field->message_type() == descriptor_) {
      capture_last_tag = true;
      break;
    }
  }

  printer->Print("::std::pair<::google::protobuf::uint32, bool> p = "
                 "input->ReadTagWithCutoffNoLastTag($max$u);\n"
                 "tag = p.first;\n"
                 "if (!p.second) goto handle_unusual;\n",
                 "max", SimpleItoa(maxtag <= kCutoff0 ? kCutoff0 :
                                   (maxtag <= kCutoff1 ? kCutoff1 :
                                    maxtag)));

  if (descriptor_->field_count() > 0) {
    // We don't even want to print the switch() if we have no fields because
    // MSVC dislikes switch() statements that contain only a default value.

    // Note:  If we just switched on the tag rather than the field number, we
    // could avoid the need for the if() to check the wire type at the beginning
    // of each case.  However, this is actually a bit slower in practice as it
    // creates a jump table that is 8x larger and sparser, and meanwhile the
    // if()s are highly predictable.
    //
    // Historically, we inserted checks to peek at the next tag on the wire and
    // jump directly to the next case statement.  While this avoids the jump
    // table that the switch uses, it greatly increases code size (20-60%) and
    // inserts branches that may fail (especially for real world protos that
    // interleave--in field number order--hot and cold fields).  Loadtests
    // confirmed that removing this optimization is performance neutral.
    printer->Print("switch (::google::protobuf::internal::WireFormatLite::"
                   "GetTagFieldNumber(tag)) {\n");

    printer->Indent();

    for (int i = 0; i < ordered_fields.size(); i++) {
      const FieldDescriptor* field = ordered_fields[i];

      PrintFieldComment(printer, field);

      printer->Print(
        "case $number$: {\n",
        "number", SimpleItoa(field->number()));
      printer->Indent();
      const FieldGenerator& field_generator = field_generators_.get(field);

      // Emit code to parse the common, expected case.
      printer->Print(
        "if (static_cast< ::google::protobuf::uint8>(tag) ==\n"
        "    static_cast< ::google::protobuf::uint8>($truncated$u /* $full$ & 0xFF */)) {\n",
        "truncated", SimpleItoa(WireFormat::MakeTag(field) & 0xFF),
        "full", SimpleItoa(WireFormat::MakeTag(field)));

      printer->Indent();
      if (field->is_packed()) {
        field_generator.GenerateMergeFromCodedStreamWithPacking(printer);
      } else {
        field_generator.GenerateMergeFromCodedStream(printer);
      }
      printer->Outdent();

      // Emit code to parse unexpectedly packed or unpacked values.
      if (field->is_packed()) {
        internal::WireFormatLite::WireType wiretype =
            WireFormat::WireTypeForFieldType(field->type());
        const uint32 tag = internal::WireFormatLite::MakeTag(
            field->number(), wiretype);
        printer->Print(
            "} else if (\n"
            "    static_cast< ::google::protobuf::uint8>(tag) ==\n"
            "    static_cast< ::google::protobuf::uint8>($truncated$u /* $full$ & 0xFF */)) {\n",
            "truncated", SimpleItoa(tag & 0xFF),
            "full", SimpleItoa(tag));

        printer->Indent();
        field_generator.GenerateMergeFromCodedStream(printer);
        printer->Outdent();
      } else if (field->is_packable() && !field->is_packed()) {
        internal::WireFormatLite::WireType wiretype =
            internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED;
         const uint32 tag = internal::WireFormatLite::MakeTag(
            field->number(), wiretype);

        printer->Print(
            "} else if (\n"
            "    static_cast< ::google::protobuf::uint8>(tag) ==\n"
            "    static_cast< ::google::protobuf::uint8>($truncated$u /* $full$ & 0xFF */)) {\n",
            "truncated", SimpleItoa(tag & 0xFF),
            "full", SimpleItoa(tag));
        printer->Indent();
        field_generator.GenerateMergeFromCodedStreamWithPacking(printer);
        printer->Outdent();
      }

      printer->Print(
        "} else {\n"
        "  goto handle_unusual;\n"
        "}\n");

      printer->Print(
        "break;\n");

      printer->Outdent();
      printer->Print("}\n\n");
    }
    printer->Print("default: {\n");
    printer->Indent();
  }

  printer->Outdent();
  printer->Print("handle_unusual:\n");
  printer->Indent();
  // If tag is 0 or an end-group tag then this must be the end of the message.
  if (capture_last_tag) {
    printer->Print(
      "if (tag == 0 ||\n"
      "    ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==\n"
      "    ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {\n"
      "  input->SetLastTag(tag);\n"
      "  goto success;\n"
      "}\n");
  } else {
    printer->Print(
      "if (tag == 0) {\n"
      "  goto success;\n"
      "}\n");
  }

  // Handle extension ranges.
  if (descriptor_->extension_range_count() > 0) {
    printer->Print(
      "if (");
    for (int i = 0; i < descriptor_->extension_range_count(); i++) {
      const Descriptor::ExtensionRange* range =
        descriptor_->extension_range(i);
      if (i > 0) printer->Print(" ||\n    ");

      uint32 start_tag = WireFormatLite::MakeTag(
        range->start, static_cast<WireFormatLite::WireType>(0));
      uint32 end_tag = WireFormatLite::MakeTag(
        range->end, static_cast<WireFormatLite::WireType>(0));

      if (range->end > FieldDescriptor::kMaxNumber) {
        printer->Print(
          "($start$u <= tag)",
          "start", SimpleItoa(start_tag));
      } else {
        printer->Print(
          "($start$u <= tag && tag < $end$u)",
          "start", SimpleItoa(start_tag),
          "end", SimpleItoa(end_tag));
      }
    }
    printer->Print(") {\n");
    if (UseUnknownFieldSet(descriptor_->file(), options_)) {
      printer->Print(vars,
        "  DO_(_extensions_.ParseField(tag, input,\n"
        "      internal_default_instance(),\n"
        "      $mutable_unknown_fields$));\n");
    } else {
      printer->Print(
        "  DO_(_extensions_.ParseField(tag, input,\n"
        "      internal_default_instance(),\n"
        "      &unknown_fields_stream));\n");
    }
    printer->Print(
      "  continue;\n"
      "}\n");
  }

  // We really don't recognize this tag.  Skip it.
  if (UseUnknownFieldSet(descriptor_->file(), options_)) {
    printer->Print(vars,
        "DO_(::google::protobuf::internal::WireFormat::SkipField(\n"
        "      input, tag, $mutable_unknown_fields$));\n");
  } else {
    printer->Print(
        "DO_(::google::protobuf::internal::WireFormatLite::SkipField(\n"
        "    input, tag, &unknown_fields_stream));\n");
  }

  if (descriptor_->field_count() > 0) {
    printer->Print("break;\n");
    printer->Outdent();
    printer->Print("}\n");    // default:
    printer->Outdent();
    printer->Print("}\n");    // switch
  }

  printer->Outdent();
  printer->Outdent();
  printer->Print(
    "  }\n"                   // for (;;)
    "success:\n"
    "  // @@protoc_insertion_point(parse_success:$full_name$)\n"
    "  return true;\n"
    "failure:\n"
    "  // @@protoc_insertion_point(parse_failure:$full_name$)\n"
    "  return false;\n"
    "#undef DO_\n"
    "}\n", "full_name", descriptor_->full_name());
}

void MessageGenerator::GenerateSerializeOneofFields(
    io::Printer* printer, const std::vector<const FieldDescriptor*>& fields,
    bool to_array) {
  GOOGLE_CHECK(!fields.empty());
  if (fields.size() == 1) {
    GenerateSerializeOneField(printer, fields[0], to_array, -1);
    return;
  }
  // We have multiple mutually exclusive choices.  Emit a switch statement.
  const OneofDescriptor* oneof = fields[0]->containing_oneof();
  printer->Print(
    "switch ($oneofname$_case()) {\n",
    "oneofname", oneof->name());
  printer->Indent();
  for (int i = 0; i < fields.size(); i++) {
    const FieldDescriptor* field = fields[i];
    printer->Print(
      "case k$field_name$:\n",
      "field_name", UnderscoresToCamelCase(field->name(), true));
    printer->Indent();
    if (to_array) {
      field_generators_.get(field).GenerateSerializeWithCachedSizesToArray(
          printer);
    } else {
      field_generators_.get(field).GenerateSerializeWithCachedSizes(printer);
    }
    printer->Print(
      "break;\n");
    printer->Outdent();
  }
  printer->Outdent();
  // Doing nothing is an option.
  printer->Print(
    "  default: ;\n"
    "}\n");
}

void MessageGenerator::GenerateSerializeOneField(
    io::Printer* printer, const FieldDescriptor* field, bool to_array,
    int cached_has_bits_index) {
  if (!field->options().weak()) {
    // For weakfields, PrintFieldComment is called during iteration.
    PrintFieldComment(printer, field);
  }

  bool have_enclosing_if = false;
  if (field->options().weak()) {
  } else if (!field->is_repeated() && HasFieldPresence(descriptor_->file())) {
    // Attempt to use the state of cached_has_bits, if possible.
    int has_bit_index = has_bit_indices_[field->index()];
    if (cached_has_bits_index == has_bit_index / 32) {
      const string mask = StrCat(
          strings::Hex(1u << (has_bit_index % 32),
          strings::ZERO_PAD_8));

      printer->Print(
          "if (cached_has_bits & 0x$mask$u) {\n", "mask", mask);
    } else {
      printer->Print(
        "if (has_$name$()) {\n",
        "name", FieldName(field));
    }

    printer->Indent();
    have_enclosing_if = true;
  } else if (!HasFieldPresence(descriptor_->file())) {
    have_enclosing_if = EmitFieldNonDefaultCondition(printer, "this->", field);
  }

  if (to_array) {
    field_generators_.get(field).GenerateSerializeWithCachedSizesToArray(
        printer);
  } else {
    field_generators_.get(field).GenerateSerializeWithCachedSizes(printer);
  }

  if (have_enclosing_if) {
    printer->Outdent();
    printer->Print("}\n");
  }
  printer->Print("\n");
}

void MessageGenerator::GenerateSerializeOneExtensionRange(
    io::Printer* printer, const Descriptor::ExtensionRange* range,
    bool to_array) {
  std::map<string, string> vars;
  vars["start"] = SimpleItoa(range->start);
  vars["end"] = SimpleItoa(range->end);
  printer->Print(vars,
    "// Extension range [$start$, $end$)\n");
  if (to_array) {
    printer->Print(vars,
      "target = _extensions_.InternalSerializeWithCachedSizesToArray(\n"
      "    $start$, $end$, deterministic, target);\n\n");
  } else {
    printer->Print(vars,
      "_extensions_.SerializeWithCachedSizes(\n"
      "    $start$, $end$, output);\n\n");
  }
}

void MessageGenerator::
GenerateSerializeWithCachedSizes(io::Printer* printer) {
  if (descriptor_->options().message_set_wire_format()) {
    // Special-case MessageSet.
    printer->Print(
      "void $classname$::SerializeWithCachedSizes(\n"
      "    ::google::protobuf::io::CodedOutputStream* output) const {\n"
      "  _extensions_.SerializeMessageSetWithCachedSizes(output);\n",
      "classname", classname_);
    GOOGLE_CHECK(UseUnknownFieldSet(descriptor_->file(), options_));
    std::map<string, string> vars;
    SetUnknkownFieldsVariable(descriptor_, options_, &vars);
    printer->Print(vars,
      "  ::google::protobuf::internal::WireFormat::SerializeUnknownMessageSetItems(\n"
      "      $unknown_fields$, output);\n");
    printer->Print(
      "}\n");
    return;
  }
  if (options_.table_driven_serialization) return;

  printer->Print(
    "void $classname$::SerializeWithCachedSizes(\n"
    "    ::google::protobuf::io::CodedOutputStream* output) const {\n",
    "classname", classname_);
  printer->Indent();

  printer->Print(
    "// @@protoc_insertion_point(serialize_start:$full_name$)\n",
    "full_name", descriptor_->full_name());

  GenerateSerializeWithCachedSizesBody(printer, false);

  printer->Print(
    "// @@protoc_insertion_point(serialize_end:$full_name$)\n",
    "full_name", descriptor_->full_name());

  printer->Outdent();
  printer->Print(
    "}\n");
}

void MessageGenerator::
GenerateSerializeWithCachedSizesToArray(io::Printer* printer) {
  if (descriptor_->options().message_set_wire_format()) {
    // Special-case MessageSet.
    printer->Print(
      "::google::protobuf::uint8* $classname$::InternalSerializeWithCachedSizesToArray(\n"
      "    bool deterministic, ::google::protobuf::uint8* target) const {\n"
      "  target = _extensions_."
      "InternalSerializeMessageSetWithCachedSizesToArray(\n"
      "               deterministic, target);\n",
      "classname", classname_);
    GOOGLE_CHECK(UseUnknownFieldSet(descriptor_->file(), options_));
    std::map<string, string> vars;
    SetUnknkownFieldsVariable(descriptor_, options_, &vars);
    printer->Print(vars,
      "  target = ::google::protobuf::internal::WireFormat::\n"
      "             SerializeUnknownMessageSetItemsToArray(\n"
      "               $unknown_fields$, target);\n");
    printer->Print(
      "  return target;\n"
      "}\n");
    return;
  }

  printer->Print(
    "::google::protobuf::uint8* $classname$::InternalSerializeWithCachedSizesToArray(\n"
    "    bool deterministic, ::google::protobuf::uint8* target) const {\n",
    "classname", classname_);
  printer->Indent();

  printer->Print("(void)deterministic; // Unused\n");
  printer->Print(
    "// @@protoc_insertion_point(serialize_to_array_start:$full_name$)\n",
    "full_name", descriptor_->full_name());

  GenerateSerializeWithCachedSizesBody(printer, true);

  printer->Print(
    "// @@protoc_insertion_point(serialize_to_array_end:$full_name$)\n",
    "full_name", descriptor_->full_name());

  printer->Outdent();
  printer->Print(
    "  return target;\n"
    "}\n");
}

void MessageGenerator::
GenerateSerializeWithCachedSizesBody(io::Printer* printer, bool to_array) {
  // If there are multiple fields in a row from the same oneof then we
  // coalesce them and emit a switch statement.  This is more efficient
  // because it lets the C++ compiler know this is a "at most one can happen"
  // situation. If we emitted "if (has_x()) ...; if (has_y()) ..." the C++
  // compiler's emitted code might check has_y() even when has_x() is true.
  class LazySerializerEmitter {
   public:
    LazySerializerEmitter(MessageGenerator* mg, io::Printer* printer,
                          bool to_array)
        : mg_(mg),
          printer_(printer),
          to_array_(to_array),
          eager_(!HasFieldPresence(mg->descriptor_->file())),
          cached_has_bit_index_(-1) {}

    ~LazySerializerEmitter() { Flush(); }

    // If conditions allow, try to accumulate a run of fields from the same
    // oneof, and handle them at the next Flush().
    void Emit(const FieldDescriptor* field) {
      if (eager_ || MustFlush(field)) {
        Flush();
      }
      if (field->containing_oneof() == NULL) {
        // TODO(ckennelly): Defer non-oneof fields similarly to oneof fields.

        if (!field->options().weak() && !field->is_repeated() && !eager_) {
          // We speculatively load the entire _has_bits_[index] contents, even
          // if it is for only one field.  Deferring non-oneof emitting would
          // allow us to determine whether this is going to be useful.
          int has_bit_index = mg_->has_bit_indices_[field->index()];
          if (cached_has_bit_index_ != has_bit_index / 32) {
            // Reload.
            int new_index = has_bit_index / 32;

            printer_->Print(
                "cached_has_bits = _has_bits_[$new_index$];\n",
                "new_index", SimpleItoa(new_index));

            cached_has_bit_index_ = new_index;
          }
        }

        mg_->GenerateSerializeOneField(
            printer_, field, to_array_, cached_has_bit_index_);
      } else {
        v_.push_back(field);
      }
    }

    void Flush() {
      if (!v_.empty()) {
        mg_->GenerateSerializeOneofFields(printer_, v_, to_array_);
        v_.clear();
      }
    }

   private:
    // If we have multiple fields in v_ then they all must be from the same
    // oneof.  Would adding field to v_ break that invariant?
    bool MustFlush(const FieldDescriptor* field) {
      return !v_.empty() &&
             v_[0]->containing_oneof() != field->containing_oneof();
    }

    MessageGenerator* mg_;
    io::Printer* printer_;
    const bool to_array_;
    const bool eager_;
    std::vector<const FieldDescriptor*> v_;

    // cached_has_bit_index_ maintains that:
    //   cached_has_bits = from._has_bits_[cached_has_bit_index_]
    // for cached_has_bit_index_ >= 0
    int cached_has_bit_index_;
  };

  std::vector<const FieldDescriptor*> ordered_fields =
      SortFieldsByNumber(descriptor_);

  std::vector<const Descriptor::ExtensionRange*> sorted_extensions;
  for (int i = 0; i < descriptor_->extension_range_count(); ++i) {
    sorted_extensions.push_back(descriptor_->extension_range(i));
  }
  std::sort(sorted_extensions.begin(), sorted_extensions.end(),
            ExtensionRangeSorter());
  if (num_weak_fields_) {
    printer->Print(
        "::google::protobuf::internal::WeakFieldMap::FieldWriter field_writer("
        "_weak_field_map_);\n");
  }

  printer->Print(
      "::google::protobuf::uint32 cached_has_bits = 0;\n"
      "(void) cached_has_bits;\n\n");

  // Merge the fields and the extension ranges, both sorted by field number.
  {
    LazySerializerEmitter e(this, printer, to_array);
    const FieldDescriptor* last_weak_field = nullptr;
    int i, j;
    for (i = 0, j = 0;
         i < ordered_fields.size() || j < sorted_extensions.size();) {
      if ((j == sorted_extensions.size()) ||
          (i < descriptor_->field_count() &&
           ordered_fields[i]->number() < sorted_extensions[j]->start)) {
        const FieldDescriptor* field = ordered_fields[i++];
        if (field->options().weak()) {
          last_weak_field = field;
          PrintFieldComment(printer, field);
        } else {
          if (last_weak_field != nullptr) {
            e.Emit(last_weak_field);
            last_weak_field = nullptr;
          }
          e.Emit(field);
        }
      } else  {
        if (last_weak_field != nullptr) {
          e.Emit(last_weak_field);
          last_weak_field = nullptr;
        }
        e.Flush();
        GenerateSerializeOneExtensionRange(printer,
                                           sorted_extensions[j++],
                                           to_array);
      }
    }
    if (last_weak_field != nullptr) {
      e.Emit(last_weak_field);
    }
  }

  std::map<string, string> vars;
  SetUnknkownFieldsVariable(descriptor_, options_, &vars);
  if (UseUnknownFieldSet(descriptor_->file(), options_)) {
    printer->Print(vars,
      "if ($have_unknown_fields$) {\n");
    printer->Indent();
    if (to_array) {
      printer->Print(vars,
        "target = "
        "::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(\n"
        "    $unknown_fields$, target);\n");
    } else {
      printer->Print(vars,
        "::google::protobuf::internal::WireFormat::SerializeUnknownFields(\n"
        "    $unknown_fields$, output);\n");
    }
    printer->Outdent();

    printer->Print("}\n");
  } else {
    printer->Print(vars,
      "output->WriteRaw($unknown_fields$.data(),\n"
      "                 static_cast<int>($unknown_fields$.size()));\n");
  }
}

std::vector<uint32> MessageGenerator::RequiredFieldsBitMask() const {
  const int array_size = HasBitsSize();
  std::vector<uint32> masks(array_size, 0);

  for (int i = 0; i < descriptor_->field_count(); i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (!field->is_required()) {
      continue;
    }

    const int has_bit_index = has_bit_indices_[field->index()];
    masks[has_bit_index / 32] |=
        static_cast<uint32>(1) << (has_bit_index % 32);
  }
  return masks;
}

// Create an expression that evaluates to
//  "for all i, (_has_bits_[i] & masks[i]) == masks[i]"
// masks is allowed to be shorter than _has_bits_, but at least one element of
// masks must be non-zero.
static string ConditionalToCheckBitmasks(const std::vector<uint32>& masks) {
  std::vector<string> parts;
  for (int i = 0; i < masks.size(); i++) {
    if (masks[i] == 0) continue;
    string m = StrCat("0x", strings::Hex(masks[i], strings::ZERO_PAD_8));
    // Each xor evaluates to 0 if the expected bits are present.
    parts.push_back(StrCat("((_has_bits_[", i, "] & ", m, ") ^ ", m, ")"));
  }
  GOOGLE_CHECK(!parts.empty());
  // If we have multiple parts, each expected to be 0, then bitwise-or them.
  string result = parts.size() == 1
                      ? parts[0]
                      : StrCat("(", Join(parts, "\n       | "), ")");
  return result + " == 0";
}

void MessageGenerator::
GenerateByteSize(io::Printer* printer) {
  if (descriptor_->options().message_set_wire_format()) {
    // Special-case MessageSet.
    GOOGLE_CHECK(UseUnknownFieldSet(descriptor_->file(), options_));
    std::map<string, string> vars;
    SetUnknkownFieldsVariable(descriptor_, options_, &vars);
    vars["classname"] = classname_;
    vars["full_name"] = descriptor_->full_name();
    printer->Print(
        vars,
        "size_t $classname$::ByteSizeLong() const {\n"
        "// @@protoc_insertion_point(message_set_byte_size_start:$full_name$)\n"
        "  size_t total_size = _extensions_.MessageSetByteSize();\n"
        "  if ($have_unknown_fields$) {\n"
        "    total_size += ::google::protobuf::internal::WireFormat::\n"
        "        ComputeUnknownMessageSetItemsSize($unknown_fields$);\n"
        "  }\n"
        "  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);\n"
        "  SetCachedSize(cached_size);\n"
        "  return total_size;\n"
        "}\n");
    return;
  }

  if (num_required_fields_ > 1 && HasFieldPresence(descriptor_->file())) {
    // Emit a function (rarely used, we hope) that handles the required fields
    // by checking for each one individually.
    printer->Print(
        "size_t $classname$::RequiredFieldsByteSizeFallback() const {\n"
        "// @@protoc_insertion_point(required_fields_byte_size_fallback_start:"
        "$full_name$)\n",
        "classname", classname_, "full_name", descriptor_->full_name());
    printer->Indent();
    printer->Print("size_t total_size = 0;\n");
    for (int i = 0; i < optimized_order_.size(); i++) {
      const FieldDescriptor* field = optimized_order_[i];
      if (field->is_required()) {
        printer->Print("\n"
                       "if (has_$name$()) {\n",
                       "name", FieldName(field));
        printer->Indent();
        PrintFieldComment(printer, field);
        field_generators_.get(field).GenerateByteSize(printer);
        printer->Outdent();
        printer->Print("}\n");
      }
    }
    printer->Print("\n"
                   "return total_size;\n");
    printer->Outdent();
    printer->Print("}\n");
  }

  printer->Print(
      "size_t $classname$::ByteSizeLong() const {\n"
      "// @@protoc_insertion_point(message_byte_size_start:$full_name$)\n",
      "classname", classname_, "full_name", descriptor_->full_name());
  printer->Indent();
  printer->Print(
    "size_t total_size = 0;\n"
    "\n");

  if (descriptor_->extension_range_count() > 0) {
    printer->Print(
      "total_size += _extensions_.ByteSize();\n"
      "\n");
  }

  std::map<string, string> vars;
  SetUnknkownFieldsVariable(descriptor_, options_, &vars);
  if (UseUnknownFieldSet(descriptor_->file(), options_)) {
    printer->Print(vars,
      "if ($have_unknown_fields$) {\n"
      "  total_size +=\n"
      "    ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(\n"
      "      $unknown_fields$);\n"
      "}\n");
  } else {
    printer->Print(vars,
      "total_size += $unknown_fields$.size();\n"
      "\n");
  }

  // Handle required fields (if any).  We expect all of them to be
  // present, so emit one conditional that checks for that.  If they are all
  // present then the fast path executes; otherwise the slow path executes.
  if (num_required_fields_ > 1 && HasFieldPresence(descriptor_->file())) {
    // The fast path works if all required fields are present.
    const std::vector<uint32> masks_for_has_bits = RequiredFieldsBitMask();
    printer->Print((string("if (") +
                    ConditionalToCheckBitmasks(masks_for_has_bits) +
                    ") {  // All required fields are present.\n").c_str());
    printer->Indent();
    // Oneof fields cannot be required, so optimized_order_ contains all of the
    // fields that we need to potentially emit.
    for (int i = 0; i < optimized_order_.size(); i++) {
      const FieldDescriptor* field = optimized_order_[i];
      if (!field->is_required()) continue;
      PrintFieldComment(printer, field);
      field_generators_.get(field).GenerateByteSize(printer);
      printer->Print("\n");
    }
    printer->Outdent();
    printer->Print("} else {\n"  // the slow path
                   "  total_size += RequiredFieldsByteSizeFallback();\n"
                   "}\n");
  } else {
    // num_required_fields_ <= 1: no need to be tricky
    for (int i = 0; i < optimized_order_.size(); i++) {
      const FieldDescriptor* field = optimized_order_[i];
      if (!field->is_required()) continue;
      PrintFieldComment(printer, field);
      printer->Print("if (has_$name$()) {\n",
                     "name", FieldName(field));
      printer->Indent();
      field_generators_.get(field).GenerateByteSize(printer);
      printer->Outdent();
      printer->Print("}\n");
    }
  }

  std::vector<std::vector<const FieldDescriptor*> > chunks = CollectFields(
      optimized_order_,
      MatchRepeatedAndHasByteAndRequired(
          &has_bit_indices_, HasFieldPresence(descriptor_->file())));

  // Remove chunks with required fields.
  chunks.erase(std::remove_if(chunks.begin(), chunks.end(), IsRequired),
               chunks.end());

  for (int chunk_index = 0; chunk_index < chunks.size(); chunk_index++) {
    const std::vector<const FieldDescriptor*>& chunk = chunks[chunk_index];
    GOOGLE_CHECK(!chunk.empty());

    // Handle repeated fields.
    if (chunk.front()->is_repeated()) {
      for (int i = 0; i < chunk.size(); i++) {
        const FieldDescriptor* field = chunk[i];

        PrintFieldComment(printer, field);
        const FieldGenerator& generator = field_generators_.get(field);
        generator.GenerateByteSize(printer);
        printer->Print("\n");
      }
      continue;
    }

    // Handle optional (non-repeated/oneof) fields.
    //
    // These are handled in chunks of 8.  The first chunk is
    // the non-requireds-non-repeateds-non-unions-non-extensions in
    //  descriptor_->field(0), descriptor_->field(1), ... descriptor_->field(7),
    // and the second chunk is the same for
    //  descriptor_->field(8), descriptor_->field(9), ...
    //  descriptor_->field(15),
    // etc.
    int last_chunk = HasFieldPresence(descriptor_->file())
                         ? has_bit_indices_[chunk.front()->index()] / 8
                         : 0;
    GOOGLE_DCHECK_NE(-1, last_chunk);

    const bool have_outer_if =
        HasFieldPresence(descriptor_->file()) && chunk.size() > 1;

    if (have_outer_if) {
      uint32 last_chunk_mask = GenChunkMask(chunk, has_bit_indices_);
      const int count = popcnt(last_chunk_mask);

      // Check (up to) 8 has_bits at a time if we have more than one field in
      // this chunk.  Due to field layout ordering, we may check
      // _has_bits_[last_chunk * 8 / 32] multiple times.
      GOOGLE_DCHECK_LE(2, count);
      GOOGLE_DCHECK_GE(8, count);

      printer->Print("if (_has_bits_[$index$ / 32] & $mask$u) {\n", "index",
                     SimpleItoa(last_chunk * 8), "mask",
                     SimpleItoa(last_chunk_mask));
      printer->Indent();
    }

    // Go back and emit checks for each of the fields we processed.
    for (int j = 0; j < chunk.size(); j++) {
      const FieldDescriptor* field = chunk[j];
      const FieldGenerator& generator = field_generators_.get(field);

      PrintFieldComment(printer, field);

      bool have_enclosing_if = false;
      if (HasFieldPresence(descriptor_->file())) {
        printer->Print("if (has_$name$()) {\n", "name", FieldName(field));
        printer->Indent();
        have_enclosing_if = true;
      } else {
        // Without field presence: field is serialized only if it has a
        // non-default value.
        have_enclosing_if =
            EmitFieldNonDefaultCondition(printer, "this->", field);
      }

      generator.GenerateByteSize(printer);

      if (have_enclosing_if) {
        printer->Outdent();
        printer->Print(
            "}\n"
            "\n");
      }
    }

    if (have_outer_if) {
      printer->Outdent();
      printer->Print("}\n");
    }
  }

  // Fields inside a oneof don't use _has_bits_ so we count them in a separate
  // pass.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    printer->Print(
        "switch ($oneofname$_case()) {\n",
        "oneofname", descriptor_->oneof_decl(i)->name());
    printer->Indent();
    for (int j = 0; j < descriptor_->oneof_decl(i)->field_count(); j++) {
      const FieldDescriptor* field = descriptor_->oneof_decl(i)->field(j);
      PrintFieldComment(printer, field);
      printer->Print(
          "case k$field_name$: {\n",
          "field_name", UnderscoresToCamelCase(field->name(), true));
      printer->Indent();
      field_generators_.get(field).GenerateByteSize(printer);
      printer->Print(
          "break;\n");
      printer->Outdent();
      printer->Print(
          "}\n");
    }
    printer->Print(
        "case $cap_oneof_name$_NOT_SET: {\n"
        "  break;\n"
        "}\n",
        "cap_oneof_name",
        ToUpper(descriptor_->oneof_decl(i)->name()));
    printer->Outdent();
    printer->Print(
        "}\n");
  }

  if (num_weak_fields_) {
    // TagSize + MessageSize
    printer->Print("total_size += _weak_field_map_.ByteSizeLong();\n");
  }

  // We update _cached_size_ even though this is a const method.  Because
  // const methods might be called concurrently this needs to be atomic
  // operations or the program is undefined.  In practice, since any concurrent
  // writes will be writing the exact same value, normal writes will work on
  // all common processors. We use a dedicated wrapper class to abstract away
  // the underlying atomic. This makes it easier on platforms where even relaxed
  // memory order might have perf impact to replace it with ordinary loads and
  // stores.
  printer->Print(
      "int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);\n"
      "SetCachedSize(cached_size);\n"
      "return total_size;\n");

  printer->Outdent();
  printer->Print("}\n");
}

void MessageGenerator::
GenerateIsInitialized(io::Printer* printer) {
  printer->Print(
    "bool $classname$::IsInitialized() const {\n",
    "classname", classname_);
  printer->Indent();

  if (descriptor_->extension_range_count() > 0) {
    printer->Print(
      "if (!_extensions_.IsInitialized()) {\n"
      "  return false;\n"
      "}\n\n");
  }

  if (HasFieldPresence(descriptor_->file())) {
    // Check that all required fields in this message are set.  We can do this
    // most efficiently by checking 32 "has bits" at a time.
    const std::vector<uint32> masks = RequiredFieldsBitMask();

    for (int i = 0; i < masks.size(); i++) {
      uint32 mask = masks[i];
      if (mask == 0) {
        continue;
      }

      // TODO(ckennelly): Consider doing something similar to ByteSizeLong(),
      // where we check all of the required fields in a single branch (assuming
      // that we aren't going to benefit from early termination).
      printer->Print(
        "if ((_has_bits_[$i$] & 0x$mask$) != 0x$mask$) return false;\n",
        "i", SimpleItoa(i),
        "mask", StrCat(strings::Hex(mask, strings::ZERO_PAD_8)));
    }
  }

  // Now check that all non-oneof embedded messages are initialized.
  for (int i = 0; i < optimized_order_.size(); i++) {
    const FieldDescriptor* field = optimized_order_[i];
    // TODO(ckennelly): Push this down into a generator?
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
        !ShouldIgnoreRequiredFieldCheck(field, options_) &&
        scc_analyzer_->HasRequiredFields(field->message_type())) {
      if (field->is_repeated()) {
        if (IsImplicitWeakField(field, options_, scc_analyzer_)) {
          printer->Print(
            "if (!::google::protobuf::internal::AllAreInitializedWeak(this->$name$_))"
            " return false;\n",
            "name", FieldName(field));
        } else {
          printer->Print(
            "if (!::google::protobuf::internal::AllAreInitialized(this->$name$()))"
            " return false;\n",
            "name", FieldName(field));
        }
      } else if (field->options().weak()) {
        continue;
      } else {
        GOOGLE_CHECK(!field->containing_oneof());
        printer->Print(
            "if (has_$name$()) {\n"
            "  if (!this->$name$_->IsInitialized()) return false;\n"
            "}\n",
            "name", FieldName(field));
      }
    }
  }
  if (num_weak_fields_) {
    // For Weak fields.
    printer->Print("if (!_weak_field_map_.IsInitialized()) return false;\n");
  }
  // Go through the oneof fields, emitting a switch if any might have required
  // fields.
  for (int i = 0; i < descriptor_->oneof_decl_count(); i++) {
    const OneofDescriptor* oneof = descriptor_->oneof_decl(i);

    bool has_required_fields = false;
    for (int j = 0; j < oneof->field_count(); j++) {
      const FieldDescriptor* field = oneof->field(j);

      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
          !ShouldIgnoreRequiredFieldCheck(field, options_) &&
          scc_analyzer_->HasRequiredFields(field->message_type())) {
        has_required_fields = true;
        break;
      }
    }

    if (!has_required_fields) {
      continue;
    }

    printer->Print(
        "switch ($oneofname$_case()) {\n",
        "oneofname", oneof->name());
    printer->Indent();
    for (int j = 0; j < oneof->field_count(); j++) {
      const FieldDescriptor* field = oneof->field(j);
      printer->Print(
          "case k$field_name$: {\n",
          "field_name", UnderscoresToCamelCase(field->name(), true));
      printer->Indent();

      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
          !ShouldIgnoreRequiredFieldCheck(field, options_) &&
          scc_analyzer_->HasRequiredFields(field->message_type())) {
        GOOGLE_CHECK(!(field->options().weak() || !field->containing_oneof()));
        if (field->options().weak()) {
          // Just skip.
        } else {
          printer->Print(
            "if (has_$name$()) {\n"
            "  if (!this->$name$().IsInitialized()) return false;\n"
            "}\n",
            "name", FieldName(field));
        }
      }

      printer->Print(
          "break;\n");
      printer->Outdent();
      printer->Print(
          "}\n");
    }
    printer->Print(
        "case $cap_oneof_name$_NOT_SET: {\n"
        "  break;\n"
        "}\n",
        "cap_oneof_name",
        ToUpper(oneof->name()));
    printer->Outdent();
    printer->Print(
        "}\n");
  }

  printer->Outdent();
  printer->Print(
    "  return true;\n"
    "}\n");
}

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

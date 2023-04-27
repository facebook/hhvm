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

#include <google/protobuf/stubs/hash.h>
#include <limits>
#include <map>
#include <queue>
#include <vector>

#include <google/protobuf/stubs/logging.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>




namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

namespace {

static const char kAnyMessageName[] = "Any";
static const char kAnyProtoFile[] = "google/protobuf/any.proto";
static const char kGoogleProtobufPrefix[] = "google/protobuf/";

string DotsToUnderscores(const string& name) {
  return StringReplace(name, ".", "_", true);
}

string DotsToColons(const string& name) {
  return StringReplace(name, ".", "::", true);
}

const char* const kKeywordList[] = {
  "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
  "bool", "break", "case", "catch", "char", "class", "compl", "const",
  "constexpr", "const_cast", "continue", "decltype", "default", "delete", "do",
  "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
  "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
  "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr",
  "operator", "or", "or_eq", "private", "protected", "public", "register",
  "reinterpret_cast", "return", "short", "signed", "sizeof", "static",
  "static_assert", "static_cast", "struct", "switch", "template", "this",
  "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
  "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t",
  "while", "xor", "xor_eq"
};

hash_set<string> MakeKeywordsMap() {
  hash_set<string> result;
  for (int i = 0; i < GOOGLE_ARRAYSIZE(kKeywordList); i++) {
    result.insert(kKeywordList[i]);
  }
  return result;
}

hash_set<string> kKeywords = MakeKeywordsMap();

// Returns whether the provided descriptor has an extension. This includes its
// nested types.
bool HasExtension(const Descriptor* descriptor) {
  if (descriptor->extension_count() > 0) {
    return true;
  }
  for (int i = 0; i < descriptor->nested_type_count(); ++i) {
    if (HasExtension(descriptor->nested_type(i))) {
      return true;
    }
  }
  return false;
}

// Encode [0..63] as 'A'-'Z', 'a'-'z', '0'-'9', '_'
char Base63Char(int value) {
  GOOGLE_CHECK_GE(value, 0);
  if (value < 26) return 'A' + value;
  value -= 26;
  if (value < 26) return 'a' + value;
  value -= 26;
  if (value < 10) return '0' + value;
  GOOGLE_CHECK_EQ(value, 10);
  return '_';
}

// Given a c identifier has 63 legal characters we can't implement base64
// encoding. So we return the k least significant "digits" in base 63.
template <typename I>
string Base63(I n, int k) {
  string res;
  while (k-- > 0) {
    res += Base63Char(static_cast<int>(n % 63));
    n /= 63;
  }
  return res;
}

}  // namespace

string UnderscoresToCamelCase(const string& input, bool cap_next_letter) {
  string result;
  // Note:  I distrust ctype.h due to locales.
  for (int i = 0; i < input.size(); i++) {
    if ('a' <= input[i] && input[i] <= 'z') {
      if (cap_next_letter) {
        result += input[i] + ('A' - 'a');
      } else {
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('A' <= input[i] && input[i] <= 'Z') {
      // Capital letters are left as-is.
      result += input[i];
      cap_next_letter = false;
    } else if ('0' <= input[i] && input[i] <= '9') {
      result += input[i];
      cap_next_letter = true;
    } else {
      cap_next_letter = true;
    }
  }
  return result;
}

const char kThickSeparator[] =
  "// ===================================================================\n";
const char kThinSeparator[] =
  "// -------------------------------------------------------------------\n";

bool CanInitializeByZeroing(const FieldDescriptor* field) {
  if (field->is_repeated() || field->is_extension()) return false;
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_ENUM:
      return field->default_value_enum()->number() == 0;
    case FieldDescriptor::CPPTYPE_INT32:
      return field->default_value_int32() == 0;
    case FieldDescriptor::CPPTYPE_INT64:
      return field->default_value_int64() == 0;
    case FieldDescriptor::CPPTYPE_UINT32:
      return field->default_value_uint32() == 0;
    case FieldDescriptor::CPPTYPE_UINT64:
      return field->default_value_uint64() == 0;
    case FieldDescriptor::CPPTYPE_FLOAT:
      return field->default_value_float() == 0;
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return field->default_value_double() == 0;
    case FieldDescriptor::CPPTYPE_BOOL:
      return field->default_value_bool() == false;
    default:
      return false;
  }
}

string ClassName(const Descriptor* descriptor) {
  const Descriptor* parent = descriptor->containing_type();
  string res;
  if (parent) res += ClassName(parent) + "_";
  res += descriptor->name();
  if (IsMapEntryMessage(descriptor)) res += "_DoNotUse";
  return res;
}

string ClassName(const EnumDescriptor* enum_descriptor) {
  if (enum_descriptor->containing_type() == NULL) {
    return enum_descriptor->name();
  } else {
    return ClassName(enum_descriptor->containing_type()) + "_" +
           enum_descriptor->name();
  }
}

string Namespace(const string& package) {
  if (package.empty()) return "";
  return "::" + DotsToColons(package);
}

string DefaultInstanceName(const Descriptor* descriptor) {
  string prefix = descriptor->file()->package().empty() ? "" : "::";
  return prefix + DotsToColons(descriptor->file()->package()) + "::_" +
      ClassName(descriptor, false) + "_default_instance_";
}

string ReferenceFunctionName(const Descriptor* descriptor) {
  return QualifiedClassName(descriptor) + "_ReferenceStrong";
}

string SuperClassName(const Descriptor* descriptor, const Options& options) {
  return HasDescriptorMethods(descriptor->file(), options)
             ? "::google::protobuf::Message"
             : "::google::protobuf::MessageLite";
}

string FieldName(const FieldDescriptor* field) {
  string result = field->name();
  LowerString(&result);
  if (kKeywords.count(result) > 0) {
    result.append("_");
  }
  return result;
}

string EnumValueName(const EnumValueDescriptor* enum_value) {
  string result = enum_value->name();
  if (kKeywords.count(result) > 0) {
    result.append("_");
  }
  return result;
}

int EstimateAlignmentSize(const FieldDescriptor* field) {
  if (field == NULL) return 0;
  if (field->is_repeated()) return 8;
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_BOOL:
      return 1;

    case FieldDescriptor::CPPTYPE_INT32:
    case FieldDescriptor::CPPTYPE_UINT32:
    case FieldDescriptor::CPPTYPE_ENUM:
    case FieldDescriptor::CPPTYPE_FLOAT:
      return 4;

    case FieldDescriptor::CPPTYPE_INT64:
    case FieldDescriptor::CPPTYPE_UINT64:
    case FieldDescriptor::CPPTYPE_DOUBLE:
    case FieldDescriptor::CPPTYPE_STRING:
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return 8;
  }
  GOOGLE_LOG(FATAL) << "Can't get here.";
  return -1;  // Make compiler happy.
}

string FieldConstantName(const FieldDescriptor *field) {
  string field_name = UnderscoresToCamelCase(field->name(), true);
  string result = "k" + field_name + "FieldNumber";

  if (!field->is_extension() &&
      field->containing_type()->FindFieldByCamelcaseName(
        field->camelcase_name()) != field) {
    // This field's camelcase name is not unique.  As a hack, add the field
    // number to the constant name.  This makes the constant rather useless,
    // but what can we do?
    result += "_" + SimpleItoa(field->number());
  }

  return result;
}

string FieldMessageTypeName(const FieldDescriptor* field) {
  // Note:  The Google-internal version of Protocol Buffers uses this function
  //   as a hook point for hacks to support legacy code.
  return ClassName(field->message_type(), true);
}

string StripProto(const string& filename) {
  if (HasSuffixString(filename, ".protodevel")) {
    return StripSuffixString(filename, ".protodevel");
  } else {
    return StripSuffixString(filename, ".proto");
  }
}

const char* PrimitiveTypeName(FieldDescriptor::CppType type) {
  switch (type) {
    case FieldDescriptor::CPPTYPE_INT32  : return "::google::protobuf::int32";
    case FieldDescriptor::CPPTYPE_INT64  : return "::google::protobuf::int64";
    case FieldDescriptor::CPPTYPE_UINT32 : return "::google::protobuf::uint32";
    case FieldDescriptor::CPPTYPE_UINT64 : return "::google::protobuf::uint64";
    case FieldDescriptor::CPPTYPE_DOUBLE : return "double";
    case FieldDescriptor::CPPTYPE_FLOAT  : return "float";
    case FieldDescriptor::CPPTYPE_BOOL   : return "bool";
    case FieldDescriptor::CPPTYPE_ENUM   : return "int";
    case FieldDescriptor::CPPTYPE_STRING : return "::std::string";
    case FieldDescriptor::CPPTYPE_MESSAGE: return NULL;

    // No default because we want the compiler to complain if any new
    // CppTypes are added.
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return NULL;
}

const char* DeclaredTypeMethodName(FieldDescriptor::Type type) {
  switch (type) {
    case FieldDescriptor::TYPE_INT32   : return "Int32";
    case FieldDescriptor::TYPE_INT64   : return "Int64";
    case FieldDescriptor::TYPE_UINT32  : return "UInt32";
    case FieldDescriptor::TYPE_UINT64  : return "UInt64";
    case FieldDescriptor::TYPE_SINT32  : return "SInt32";
    case FieldDescriptor::TYPE_SINT64  : return "SInt64";
    case FieldDescriptor::TYPE_FIXED32 : return "Fixed32";
    case FieldDescriptor::TYPE_FIXED64 : return "Fixed64";
    case FieldDescriptor::TYPE_SFIXED32: return "SFixed32";
    case FieldDescriptor::TYPE_SFIXED64: return "SFixed64";
    case FieldDescriptor::TYPE_FLOAT   : return "Float";
    case FieldDescriptor::TYPE_DOUBLE  : return "Double";

    case FieldDescriptor::TYPE_BOOL    : return "Bool";
    case FieldDescriptor::TYPE_ENUM    : return "Enum";

    case FieldDescriptor::TYPE_STRING  : return "String";
    case FieldDescriptor::TYPE_BYTES   : return "Bytes";
    case FieldDescriptor::TYPE_GROUP   : return "Group";
    case FieldDescriptor::TYPE_MESSAGE : return "Message";

    // No default because we want the compiler to complain if any new
    // types are added.
  }
  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}

string Int32ToString(int number) {
  // gcc rejects the decimal form of kint32min.
  if (number == kint32min) {
    GOOGLE_COMPILE_ASSERT(kint32min == (~0x7fffffff), kint32min_value_error);
    return "(~0x7fffffff)";
  } else {
    return SimpleItoa(number);
  }
}

string Int64ToString(int64 number) {
  // gcc rejects the decimal form of kint64min
  if (number == kint64min) {
    // Make sure we are in a 2's complement system.
    GOOGLE_COMPILE_ASSERT(kint64min == GOOGLE_LONGLONG(~0x7fffffffffffffff),
                   kint64min_value_error);
    return "GOOGLE_LONGLONG(~0x7fffffffffffffff)";
  }
  return "GOOGLE_LONGLONG(" + SimpleItoa(number) + ")";
}

string DefaultValue(const FieldDescriptor* field) {
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return Int32ToString(field->default_value_int32());
    case FieldDescriptor::CPPTYPE_UINT32:
      return SimpleItoa(field->default_value_uint32()) + "u";
    case FieldDescriptor::CPPTYPE_INT64:
      return Int64ToString(field->default_value_int64());
    case FieldDescriptor::CPPTYPE_UINT64:
      return "GOOGLE_ULONGLONG(" + SimpleItoa(field->default_value_uint64())+ ")";
    case FieldDescriptor::CPPTYPE_DOUBLE: {
      double value = field->default_value_double();
      if (value == std::numeric_limits<double>::infinity()) {
        return "::google::protobuf::internal::Infinity()";
      } else if (value == -std::numeric_limits<double>::infinity()) {
        return "-::google::protobuf::internal::Infinity()";
      } else if (value != value) {
        return "::google::protobuf::internal::NaN()";
      } else {
        return SimpleDtoa(value);
      }
    }
    case FieldDescriptor::CPPTYPE_FLOAT:
      {
        float value = field->default_value_float();
        if (value == std::numeric_limits<float>::infinity()) {
          return "static_cast<float>(::google::protobuf::internal::Infinity())";
        } else if (value == -std::numeric_limits<float>::infinity()) {
          return "static_cast<float>(-::google::protobuf::internal::Infinity())";
        } else if (value != value) {
          return "static_cast<float>(::google::protobuf::internal::NaN())";
        } else {
          string float_value = SimpleFtoa(value);
          // If floating point value contains a period (.) or an exponent
          // (either E or e), then append suffix 'f' to make it a float
          // literal.
          if (float_value.find_first_of(".eE") != string::npos) {
            float_value.push_back('f');
          }
          return float_value;
        }
      }
    case FieldDescriptor::CPPTYPE_BOOL:
      return field->default_value_bool() ? "true" : "false";
    case FieldDescriptor::CPPTYPE_ENUM:
      // Lazy:  Generate a static_cast because we don't have a helper function
      //   that constructs the full name of an enum value.
      return strings::Substitute(
          "static_cast< $0 >($1)",
          ClassName(field->enum_type(), true),
          Int32ToString(field->default_value_enum()->number()));
    case FieldDescriptor::CPPTYPE_STRING:
      return "\"" + EscapeTrigraphs(
        CEscape(field->default_value_string())) +
        "\"";
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "*" + FieldMessageTypeName(field) +
             "::internal_default_instance()";
  }
  // Can't actually get here; make compiler happy.  (We could add a default
  // case above but then we wouldn't get the nice compiler warning when a
  // new type is added.)
  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}

// Convert a file name into a valid identifier.
string FilenameIdentifier(const string& filename) {
  string result;
  for (int i = 0; i < filename.size(); i++) {
    if (ascii_isalnum(filename[i])) {
      result.push_back(filename[i]);
    } else {
      // Not alphanumeric.  To avoid any possibility of name conflicts we
      // use the hex code for the character.
      StrAppend(&result, "_", strings::Hex(static_cast<uint8>(filename[i])));
    }
  }
  return result;
}

string FileLevelNamespace(const string& filename) {
  return "protobuf_" + FilenameIdentifier(filename);
}

// Return the qualified C++ name for a file level symbol.
string QualifiedFileLevelSymbol(const string& package, const string& name) {
  if (package.empty()) {
    return StrCat("::", name);
  }
  return StrCat("::", DotsToColons(package), "::", name);
}

// Escape C++ trigraphs by escaping question marks to \?
string EscapeTrigraphs(const string& to_escape) {
  return StringReplace(to_escape, "?", "\\?", true);
}

// Escaped function name to eliminate naming conflict.
string SafeFunctionName(const Descriptor* descriptor,
                        const FieldDescriptor* field,
                        const string& prefix) {
  // Do not use FieldName() since it will escape keywords.
  string name = field->name();
  LowerString(&name);
  string function_name = prefix + name;
  if (descriptor->FindFieldByName(function_name)) {
    // Single underscore will also make it conflicting with the private data
    // member. We use double underscore to escape function names.
    function_name.append("__");
  } else if (kKeywords.count(name) > 0) {
    // If the field name is a keyword, we append the underscore back to keep it
    // consistent with other function names.
    function_name.append("_");
  }
  return function_name;
}


static bool HasMapFields(const Descriptor* descriptor) {
  for (int i = 0; i < descriptor->field_count(); ++i) {
    if (descriptor->field(i)->is_map()) {
      return true;
    }
  }
  for (int i = 0; i < descriptor->nested_type_count(); ++i) {
    if (HasMapFields(descriptor->nested_type(i))) return true;
  }
  return false;
}

bool HasMapFields(const FileDescriptor* file) {
  for (int i = 0; i < file->message_type_count(); ++i) {
    if (HasMapFields(file->message_type(i))) return true;
  }
  return false;
}

static bool HasEnumDefinitions(const Descriptor* message_type) {
  if (message_type->enum_type_count() > 0) return true;
  for (int i = 0; i < message_type->nested_type_count(); ++i) {
    if (HasEnumDefinitions(message_type->nested_type(i))) return true;
  }
  return false;
}

bool HasEnumDefinitions(const FileDescriptor* file) {
  if (file->enum_type_count() > 0) return true;
  for (int i = 0; i < file->message_type_count(); ++i) {
    if (HasEnumDefinitions(file->message_type(i))) return true;
  }
  return false;
}

bool IsStringOrMessage(const FieldDescriptor* field) {
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
    case FieldDescriptor::CPPTYPE_INT64:
    case FieldDescriptor::CPPTYPE_UINT32:
    case FieldDescriptor::CPPTYPE_UINT64:
    case FieldDescriptor::CPPTYPE_DOUBLE:
    case FieldDescriptor::CPPTYPE_FLOAT:
    case FieldDescriptor::CPPTYPE_BOOL:
    case FieldDescriptor::CPPTYPE_ENUM:
      return false;
    case FieldDescriptor::CPPTYPE_STRING:
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return true;
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return false;
}

FieldOptions::CType EffectiveStringCType(const FieldDescriptor* field) {
  GOOGLE_DCHECK(field->cpp_type() == FieldDescriptor::CPPTYPE_STRING);
  // Open-source protobuf release only supports STRING ctype.
  return FieldOptions::STRING;

}

bool IsAnyMessage(const FileDescriptor* descriptor) {
  return descriptor->name() == kAnyProtoFile;
}

bool IsAnyMessage(const Descriptor* descriptor) {
  return descriptor->name() == kAnyMessageName &&
         descriptor->file()->name() == kAnyProtoFile;
}

bool IsWellKnownMessage(const FileDescriptor* descriptor) {
  return !descriptor->name().compare(0, 16, kGoogleProtobufPrefix);
}

enum Utf8CheckMode {
  STRICT = 0,  // Parsing will fail if non UTF-8 data is in string fields.
  VERIFY = 1,  // Only log an error but parsing will succeed.
  NONE = 2,  // No UTF-8 check.
};

// Which level of UTF-8 enforcemant is placed on this file.
static Utf8CheckMode GetUtf8CheckMode(const FieldDescriptor* field,
                                      const Options& options) {
  if (field->file()->syntax() == FileDescriptor::SYNTAX_PROTO3) {
    return STRICT;
  } else if (GetOptimizeFor(field->file(), options) !=
             FileOptions::LITE_RUNTIME) {
    return VERIFY;
  } else {
    return NONE;
  }
}

static void GenerateUtf8CheckCode(const FieldDescriptor* field,
                                  const Options& options, bool for_parse,
                                  const std::map<string, string>& variables,
                                  const char* parameters,
                                  const char* strict_function,
                                  const char* verify_function,
                                  io::Printer* printer) {
  switch (GetUtf8CheckMode(field, options)) {
    case STRICT: {
      if (for_parse) {
        printer->Print("DO_(");
      }
      printer->Print(
          "::google::protobuf::internal::WireFormatLite::$function$(\n",
          "function", strict_function);
      printer->Indent();
      printer->Print(variables, parameters);
      if (for_parse) {
        printer->Print("::google::protobuf::internal::WireFormatLite::PARSE,\n");
      } else {
        printer->Print("::google::protobuf::internal::WireFormatLite::SERIALIZE,\n");
      }
      printer->Print("\"$full_name$\")", "full_name", field->full_name());
      if (for_parse) {
        printer->Print(")");
      }
      printer->Print(";\n");
      printer->Outdent();
      break;
    }
    case VERIFY: {
      printer->Print(
          "::google::protobuf::internal::WireFormat::$function$(\n",
          "function", verify_function);
      printer->Indent();
      printer->Print(variables, parameters);
      if (for_parse) {
        printer->Print("::google::protobuf::internal::WireFormat::PARSE,\n");
      } else {
        printer->Print("::google::protobuf::internal::WireFormat::SERIALIZE,\n");
      }
      printer->Print("\"$full_name$\");\n", "full_name", field->full_name());
      printer->Outdent();
      break;
    }
    case NONE:
      break;
  }
}

void GenerateUtf8CheckCodeForString(const FieldDescriptor* field,
                                    const Options& options, bool for_parse,
                                    const std::map<string, string>& variables,
                                    const char* parameters,
                                    io::Printer* printer) {
  GenerateUtf8CheckCode(field, options, for_parse, variables, parameters,
                        "VerifyUtf8String", "VerifyUTF8StringNamedField",
                        printer);
}

void GenerateUtf8CheckCodeForCord(const FieldDescriptor* field,
                                  const Options& options, bool for_parse,
                                  const std::map<string, string>& variables,
                                  const char* parameters,
                                  io::Printer* printer) {
  GenerateUtf8CheckCode(field, options, for_parse, variables, parameters,
                        "VerifyUtf8Cord", "VerifyUTF8CordNamedField", printer);
}

namespace {

void Flatten(const Descriptor* descriptor,
             std::vector<const Descriptor*>* flatten) {
  for (int i = 0; i < descriptor->nested_type_count(); i++)
    Flatten(descriptor->nested_type(i), flatten);
  flatten->push_back(descriptor);
}

}  // namespace

void FlattenMessagesInFile(const FileDescriptor* file,
                           std::vector<const Descriptor*>* result) {
  for (int i = 0; i < file->message_type_count(); i++) {
    Flatten(file->message_type(i), result);
  }
}

bool HasWeakFields(const Descriptor* descriptor) {
  return false;
}

bool HasWeakFields(const FileDescriptor* file) {
  return false;
}

bool UsingImplicitWeakFields(const FileDescriptor* file,
                             const Options& options) {
  return options.lite_implicit_weak_fields &&
         GetOptimizeFor(file, options) == FileOptions::LITE_RUNTIME;
}

bool IsImplicitWeakField(const FieldDescriptor* field, const Options& options,
                         SCCAnalyzer* scc_analyzer) {
  return UsingImplicitWeakFields(field->file(), options) &&
         field->type() == FieldDescriptor::TYPE_MESSAGE &&
         !field->is_required() && !field->is_map() &&
         field->containing_oneof() == NULL &&
         !IsWellKnownMessage(field->message_type()->file()) &&
         // We do not support implicit weak fields between messages in the same
         // strongly-connected component.
         scc_analyzer->GetSCC(field->containing_type()) !=
             scc_analyzer->GetSCC(field->message_type());
}

struct CompareDescriptors {
  bool operator()(const Descriptor* a, const Descriptor* b) {
    return a->full_name() < b->full_name();
  }
};

SCCAnalyzer::NodeData SCCAnalyzer::DFS(const Descriptor* descriptor) {
  // Must not have visited already.
  GOOGLE_DCHECK_EQ(cache_.count(descriptor), 0);

  // Mark visited by inserting in map.
  NodeData& result = cache_[descriptor];
  // Initialize data structures.
  result.index = result.lowlink = index_++;
  stack_.push_back(descriptor);

  // Recurse the fields / nodes in graph
  for (int i = 0; i < descriptor->field_count(); i++) {
    const Descriptor* child = descriptor->field(i)->message_type();
    if (child) {
      if (cache_.count(child) == 0) {
        // unexplored node
        NodeData child_data = DFS(child);
        result.lowlink = std::min(result.lowlink, child_data.lowlink);
      } else {
        NodeData child_data = cache_[child];
        if (child_data.scc == NULL) {
          // Still in the stack_ so we found a back edge
          result.lowlink = std::min(result.lowlink, child_data.index);
        }
      }
    }
  }
  if (result.index == result.lowlink) {
    // This is the root of a strongly connected component
    SCC* scc = CreateSCC();
    while (true) {
      const Descriptor* scc_desc = stack_.back();
      scc->descriptors.push_back(scc_desc);
      // Remove from stack
      stack_.pop_back();
      cache_[scc_desc].scc = scc;

      if (scc_desc == descriptor) break;
    }

    // The order of descriptors is random and depends how this SCC was
    // discovered. In-order to ensure maximum stability we sort it by name.
    std::sort(scc->descriptors.begin(), scc->descriptors.end(),
              CompareDescriptors());
    AddChildren(scc);
  }
  return result;
}

void SCCAnalyzer::AddChildren(SCC* scc) {
  std::set<const SCC*> seen;
  for (int i = 0; i < scc->descriptors.size(); i++) {
    const Descriptor* descriptor = scc->descriptors[i];
    for (int j = 0; j < descriptor->field_count(); j++) {
      const Descriptor* child_msg = descriptor->field(j)->message_type();
      if (child_msg) {
        const SCC* child = GetSCC(child_msg);
        if (child == scc) continue;
        if (seen.insert(child).second) {
          scc->children.push_back(child);
        }
      }
    }
  }
}

MessageAnalysis SCCAnalyzer::GetSCCAnalysis(const SCC* scc) {
  if (analysis_cache_.count(scc)) return analysis_cache_[scc];
  MessageAnalysis result = MessageAnalysis();
  for (int i = 0; i < scc->descriptors.size(); i++) {
    const Descriptor* descriptor = scc->descriptors[i];
    if (descriptor->extension_range_count() > 0) {
      result.contains_extension = true;
    }
    for (int i = 0; i < descriptor->field_count(); i++) {
      const FieldDescriptor* field = descriptor->field(i);
      if (field->is_required()) {
        result.contains_required = true;
      }
      switch (field->type()) {
        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_BYTES: {
          if (field->options().ctype() == FieldOptions::CORD) {
            result.contains_cord = true;
          }
          break;
        }
        case FieldDescriptor::TYPE_GROUP:
        case FieldDescriptor::TYPE_MESSAGE: {
          const SCC* child = GetSCC(field->message_type());
          if (child != scc) {
            MessageAnalysis analysis = GetSCCAnalysis(child);
            result.contains_cord |= analysis.contains_cord;
            result.contains_extension |= analysis.contains_extension;
            if (!ShouldIgnoreRequiredFieldCheck(field, options_)) {
              result.contains_required |= analysis.contains_required;
            }
          } else {
            // This field points back into the same SCC hence the messages
            // in the SCC are recursive. Note if SCC contains more than two
            // nodes it has to be recursive, however this test also works for
            // a single node that is recursive.
            result.is_recursive = true;
          }
          break;
        }
        default:
          break;
      }
    }
  }
  // We deliberately only insert the result here. After we contracted the SCC
  // in the graph, the graph should be a DAG. Hence we shouldn't need to mark
  // nodes visited as we can never return to them. By inserting them here
  // we will go in an infinite loop if the SCC is not correct.
  return analysis_cache_[scc] = result;
}

void ListAllFields(const Descriptor* d,
                   std::vector<const FieldDescriptor*>* fields) {
  // Collect sub messages
  for (int i = 0; i < d->nested_type_count(); i++) {
    ListAllFields(d->nested_type(i), fields);
  }
  // Collect message level extensions.
  for (int i = 0; i < d->extension_count(); i++) {
    fields->push_back(d->extension(i));
  }
  // Add types of fields necessary
  for (int i = 0; i < d->field_count(); i++) {
    fields->push_back(d->field(i));
  }
}

void ListAllFields(const FileDescriptor* d,
                   std::vector<const FieldDescriptor*>* fields) {
  // Collect file level message.
  for (int i = 0; i < d->message_type_count(); i++) {
    ListAllFields(d->message_type(i), fields);
  }
  // Collect message level extensions.
  for (int i = 0; i < d->extension_count(); i++) {
    fields->push_back(d->extension(i));
  }
}

void ListAllTypesForServices(const FileDescriptor* fd,
                             std::vector<const Descriptor*>* types) {
  for (int i = 0; i < fd->service_count(); i++) {
    const ServiceDescriptor* sd = fd->service(i);
    for (int j = 0; j < sd->method_count(); j++) {
      const MethodDescriptor* method = sd->method(j);
      types->push_back(method->input_type());
      types->push_back(method->output_type());
    }
  }
}


}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

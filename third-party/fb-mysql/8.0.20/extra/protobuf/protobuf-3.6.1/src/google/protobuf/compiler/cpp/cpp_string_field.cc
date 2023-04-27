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

#include <google/protobuf/compiler/cpp/cpp_string_field.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>

#include <google/protobuf/stubs/strutil.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

namespace {

void SetStringVariables(const FieldDescriptor* descriptor,
                        std::map<string, string>* variables,
                        const Options& options) {
  SetCommonFieldVariables(descriptor, variables, options);
  (*variables)["default"] = DefaultValue(descriptor);
  (*variables)["default_length"] =
      SimpleItoa(descriptor->default_value_string().length());
  string default_variable_string = MakeDefaultName(descriptor);
  (*variables)["default_variable_name"] = default_variable_string;
  (*variables)["default_variable"] =
      descriptor->default_value_string().empty()
          ? "&::google::protobuf::internal::GetEmptyStringAlreadyInited()"
          : "&" + Namespace(descriptor) + "::" + (*variables)["classname"] +
                "::" + default_variable_string + ".get()";
  (*variables)["pointer_type"] =
      descriptor->type() == FieldDescriptor::TYPE_BYTES ? "void" : "char";
  (*variables)["null_check"] = "GOOGLE_DCHECK(value != NULL);\n";
  // NOTE: Escaped here to unblock proto1->proto2 migration.
  // TODO(liujisi): Extend this to apply for other conflicting methods.
  (*variables)["release_name"] =
      SafeFunctionName(descriptor->containing_type(),
                       descriptor, "release_");
  (*variables)["full_name"] = descriptor->full_name();

  (*variables)["string_piece"] = "::std::string";

  (*variables)["lite"] =
      HasDescriptorMethods(descriptor->file(), options) ? "" : "Lite";
}

}  // namespace

// ===================================================================

StringFieldGenerator::StringFieldGenerator(const FieldDescriptor* descriptor,
                                           const Options& options)
    : FieldGenerator(options),
      descriptor_(descriptor),
      lite_(!HasDescriptorMethods(descriptor->file(), options)),
      inlined_(false) {

  // TODO(ckennelly): Handle inlining for any.proto.
  if (IsAnyMessage(descriptor_->containing_type())) {
    inlined_ = false;
  }
  if (descriptor_->containing_type()->options().map_entry()) {
    inlined_ = false;
  }

  // Limit to proto2, as we rely on has bits to distinguish field presence for
  // release_$name$.  On proto3, we cannot use the address of the string
  // instance when the field has been inlined.
  inlined_ = inlined_ && HasFieldPresence(descriptor_->file());

  SetStringVariables(descriptor, &variables_, options);
}

StringFieldGenerator::~StringFieldGenerator() {}

void StringFieldGenerator::
GeneratePrivateMembers(io::Printer* printer) const {
  if (inlined_) {
    printer->Print(variables_,
                   "::google::protobuf::internal::InlinedStringField $name$_;\n");
  } else {
    // N.B. that we continue to use |ArenaStringPtr| instead of |string*| for
    // string fields, even when SupportArenas(descriptor_) == false. Why?  The
    // simple answer is to avoid unmaintainable complexity. The reflection code
    // assumes ArenaStringPtrs. These are *almost* in-memory-compatible with
    // string*, except for the pointer tags and related ownership semantics. We
    // could modify the runtime code to use string* for the
    // not-supporting-arenas case, but this would require a way to detect which
    // type of class was generated (adding overhead and complexity to
    // GeneratedMessageReflection) and littering the runtime code paths with
    // conditionals. It's simpler to stick with this but use lightweight
    // accessors that assume arena == NULL.  There should be very little
    // overhead anyway because it's just a tagged pointer in-memory.
    printer->Print(variables_, "::google::protobuf::internal::ArenaStringPtr $name$_;\n");
  }
}

void StringFieldGenerator::
GenerateStaticMembers(io::Printer* printer) const {
  if (!descriptor_->default_value_string().empty()) {
    // We make the default instance public, so it can be initialized by
    // non-friend code.
    printer->Print(variables_,
                   "public:\n"
                   "static ::google::protobuf::internal::ExplicitlyConstructed< ::std::string>"
                   " $default_variable_name$;\n"
                   "private:\n");
  }
}

void StringFieldGenerator::
GenerateAccessorDeclarations(io::Printer* printer) const {
  // If we're using StringFieldGenerator for a field with a ctype, it's
  // because that ctype isn't actually implemented.  In particular, this is
  // true of ctype=CORD and ctype=STRING_PIECE in the open source release.
  // We aren't releasing Cord because it has too many Google-specific
  // dependencies and we aren't releasing StringPiece because it's hardly
  // useful outside of Google and because it would get confusing to have
  // multiple instances of the StringPiece class in different libraries (PCRE
  // already includes it for their C++ bindings, which came from Google).
  //
  // In any case, we make all the accessors private while still actually
  // using a string to represent the field internally.  This way, we can
  // guarantee that if we do ever implement the ctype, it won't break any
  // existing users who might be -- for whatever reason -- already using .proto
  // files that applied the ctype.  The field can still be accessed via the
  // reflection interface since the reflection interface is independent of
  // the string's underlying representation.

  bool unknown_ctype =
      descriptor_->options().ctype() != EffectiveStringCType(descriptor_);

  if (unknown_ctype) {
    printer->Outdent();
    printer->Print(
      " private:\n"
      "  // Hidden due to unknown ctype option.\n");
    printer->Indent();
  }

  printer->Print(variables_,
                 "$deprecated_attr$const ::std::string& $name$() const;\n");
  printer->Annotate("name", descriptor_);
  printer->Print(
      variables_,
      "$deprecated_attr$void ${$set_$name$$}$(const ::std::string& value);\n");
  printer->Annotate("{", "}", descriptor_);

  printer->Print(variables_,
                 "#if LANG_CXX11\n"
                 "$deprecated_attr$void ${$set_$name$$}$(::std::string&& value);\n"
                 "#endif\n");
  printer->Annotate("{", "}", descriptor_);

  printer->Print(
      variables_,
      "$deprecated_attr$void ${$set_$name$$}$(const char* value);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$void ${$set_$name$$}$(const $pointer_type$* "
                 "value, size_t size)"
                 ";\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$::std::string* ${$mutable_$name$$}$();\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_, "$deprecated_attr$::std::string* $release_name$();\n");
  printer->Annotate("release_name", descriptor_);
  printer->Print(
      variables_,
      "$deprecated_attr$void ${$set_allocated_$name$$}$(::std::string* $name$);\n");
  printer->Annotate("{", "}", descriptor_);
  if (SupportsArenas(descriptor_)) {
    printer->Print(
        variables_,
        "PROTOBUF_RUNTIME_DEPRECATED(\"The unsafe_arena_ accessors for\"\n"
        "\"    string fields are deprecated and will be removed in a\"\n"
        "\"    future release.\")\n"
        "::std::string* ${$unsafe_arena_release_$name$$}$();\n");
    printer->Annotate("{", "}", descriptor_);
    printer->Print(
        variables_,
        "PROTOBUF_RUNTIME_DEPRECATED(\"The unsafe_arena_ accessors for\"\n"
        "\"    string fields are deprecated and will be removed in a\"\n"
        "\"    future release.\")\n"
        "void ${$unsafe_arena_set_allocated_$name$$}$(\n"
        "    ::std::string* $name$);\n");
    printer->Annotate("{", "}", descriptor_);
  }

  if (unknown_ctype) {
    printer->Outdent();
    printer->Print(" public:\n");
    printer->Indent();
  }
}

void StringFieldGenerator::
GenerateInlineAccessorDefinitions(io::Printer* printer) const {
  if (SupportsArenas(descriptor_)) {
    printer->Print(
        variables_,
        "inline const ::std::string& $classname$::$name$() const {\n"
        "  // @@protoc_insertion_point(field_get:$full_name$)\n"
        "  return $name$_.Get();\n"
        "}\n"
        "inline void $classname$::set_$name$(const ::std::string& value) {\n"
        "  $set_hasbit$\n"
        "  $name$_.Set$lite$($default_variable$, value, GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "}\n"
        "#if LANG_CXX11\n"
        "inline void $classname$::set_$name$(::std::string&& value) {\n"
        "  $set_hasbit$\n"
        "  $name$_.Set$lite$(\n"
        "    $default_variable$, ::std::move(value), GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_rvalue:$full_name$)\n"
        "}\n"
        "#endif\n"
        "inline void $classname$::set_$name$(const char* value) {\n"
        "  $null_check$"
        "  $set_hasbit$\n"
        "  $name$_.Set$lite$($default_variable$, $string_piece$(value),\n"
        "              GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_char:$full_name$)\n"
        "}\n"
        "inline "
        "void $classname$::set_$name$(const $pointer_type$* value,\n"
        "    size_t size) {\n"
        "  $set_hasbit$\n"
        "  $name$_.Set$lite$($default_variable$, $string_piece$(\n"
        "      reinterpret_cast<const char*>(value), size), "
        "GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_pointer:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::mutable_$name$() {\n"
        "  $set_hasbit$\n"
        "  // @@protoc_insertion_point(field_mutable:$full_name$)\n"
        "  return $name$_.Mutable($default_variable$, GetArenaNoVirtual());\n"
        "}\n"
        "inline ::std::string* $classname$::$release_name$() {\n"
        "  // @@protoc_insertion_point(field_release:$full_name$)\n");

    if (HasFieldPresence(descriptor_->file())) {
      printer->Print(variables_,
        "  if (!has_$name$()) {\n"
        "    return NULL;\n"
        "  }\n"
        "  $clear_hasbit$\n"
        "  return $name$_.ReleaseNonDefault("
        "$default_variable$, GetArenaNoVirtual());\n");
    } else {
      printer->Print(variables_,
        "  $clear_hasbit$\n"
        "  return $name$_.Release($default_variable$, GetArenaNoVirtual());\n");
    }

    printer->Print(variables_,
        "}\n"
        "inline void $classname$::set_allocated_$name$(::std::string* $name$) {\n"
        "  if ($name$ != NULL) {\n"
        "    $set_hasbit$\n"
        "  } else {\n"
        "    $clear_hasbit$\n"
        "  }\n"
        "  $name$_.SetAllocated($default_variable$, $name$,\n"
        "      GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_allocated:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::unsafe_arena_release_$name$() {\n"
        "  // "
        "@@protoc_insertion_point(field_unsafe_arena_release:$full_name$)\n"
        "  GOOGLE_DCHECK(GetArenaNoVirtual() != NULL);\n"
        "  $clear_hasbit$\n"
        "  return $name$_.UnsafeArenaRelease($default_variable$,\n"
        "      GetArenaNoVirtual());\n"
        "}\n"
        "inline void $classname$::unsafe_arena_set_allocated_$name$(\n"
        "    ::std::string* $name$) {\n"
        "  GOOGLE_DCHECK(GetArenaNoVirtual() != NULL);\n"
        "  if ($name$ != NULL) {\n"
        "    $set_hasbit$\n"
        "  } else {\n"
        "    $clear_hasbit$\n"
        "  }\n"
        "  $name$_.UnsafeArenaSetAllocated($default_variable$,\n"
        "      $name$, GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:"
        "$full_name$)\n"
        "}\n");
  } else {
    // No-arena case.
    printer->Print(
        variables_,
        "inline const ::std::string& $classname$::$name$() const {\n"
        "  // @@protoc_insertion_point(field_get:$full_name$)\n"
        "  return $name$_.GetNoArena();\n"
        "}\n"
        "inline void $classname$::set_$name$(const ::std::string& value) {\n"
        "  $set_hasbit$\n"
        "  $name$_.SetNoArena($default_variable$, value);\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "}\n"
        "#if LANG_CXX11\n"
        "inline void $classname$::set_$name$(::std::string&& value) {\n"
        "  $set_hasbit$\n"
        "  $name$_.SetNoArena(\n"
        "    $default_variable$, ::std::move(value));\n"
        "  // @@protoc_insertion_point(field_set_rvalue:$full_name$)\n"
        "}\n"
        "#endif\n"
        "inline void $classname$::set_$name$(const char* value) {\n"
        "  $null_check$"
        "  $set_hasbit$\n"
        "  $name$_.SetNoArena($default_variable$, $string_piece$(value));\n"
        "  // @@protoc_insertion_point(field_set_char:$full_name$)\n"
        "}\n"
        "inline "
        "void $classname$::set_$name$(const $pointer_type$* value, "
        "size_t size) {\n"
        "  $set_hasbit$\n"
        "  $name$_.SetNoArena($default_variable$,\n"
        "      $string_piece$(reinterpret_cast<const char*>(value), size));\n"
        "  // @@protoc_insertion_point(field_set_pointer:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::mutable_$name$() {\n"
        "  $set_hasbit$\n"
        "  // @@protoc_insertion_point(field_mutable:$full_name$)\n"
        "  return $name$_.MutableNoArena($default_variable$);\n"
        "}\n"
        "inline ::std::string* $classname$::$release_name$() {\n"
        "  // @@protoc_insertion_point(field_release:$full_name$)\n");

    if (HasFieldPresence(descriptor_->file())) {
      printer->Print(variables_,
        "  if (!has_$name$()) {\n"
        "    return NULL;\n"
        "  }\n"
        "  $clear_hasbit$\n"
        "  return $name$_.ReleaseNonDefaultNoArena($default_variable$);\n");
    } else {
      printer->Print(variables_,
        "  $clear_hasbit$\n"
        "  return $name$_.ReleaseNoArena($default_variable$);\n");
    }

    printer->Print(variables_,
        "}\n"
        "inline void $classname$::set_allocated_$name$(::std::string* $name$) {\n"
        "  if ($name$ != NULL) {\n"
        "    $set_hasbit$\n"
        "  } else {\n"
        "    $clear_hasbit$\n"
        "  }\n"
        "  $name$_.SetAllocatedNoArena($default_variable$, $name$);\n"
        "  // @@protoc_insertion_point(field_set_allocated:$full_name$)\n"
        "}\n");
  }
}

void StringFieldGenerator::
GenerateNonInlineAccessorDefinitions(io::Printer* printer) const {
  if (!descriptor_->default_value_string().empty()) {
    // Initialized in GenerateDefaultInstanceAllocator.
    printer->Print(variables_,
                   "::google::protobuf::internal::ExplicitlyConstructed<::std::string> "
                   "$classname$::$default_variable_name$;\n");
  }
}

void StringFieldGenerator::
GenerateClearingCode(io::Printer* printer) const {
  // Two-dimension specialization here: supporting arenas or not, and default
  // value is the empty string or not. Complexity here ensures the minimal
  // number of branches / amount of extraneous code at runtime (given that the
  // below methods are inlined one-liners)!
  if (SupportsArenas(descriptor_)) {
    if (descriptor_->default_value_string().empty()) {
      printer->Print(variables_,
        "$name$_.ClearToEmpty($default_variable$, GetArenaNoVirtual());\n");
    } else {
      printer->Print(variables_,
        "$name$_.ClearToDefault($default_variable$, GetArenaNoVirtual());\n");
    }
  } else {
    if (descriptor_->default_value_string().empty()) {
      printer->Print(variables_,
        "$name$_.ClearToEmptyNoArena($default_variable$);\n");
    } else {
      printer->Print(variables_,
        "$name$_.ClearToDefaultNoArena($default_variable$);\n");
    }
  }
}

void StringFieldGenerator::
GenerateMessageClearingCode(io::Printer* printer) const {
  // Two-dimension specialization here: supporting arenas, field presence, or
  // not, and default value is the empty string or not. Complexity here ensures
  // the minimal number of branches / amount of extraneous code at runtime
  // (given that the below methods are inlined one-liners)!

  // If we have field presence, then the Clear() method of the protocol buffer
  // will have checked that this field is set.  If so, we can avoid redundant
  // checks against default_variable.
  const bool must_be_present =
      HasFieldPresence(descriptor_->file());

  if (inlined_ && must_be_present) {
    // Calling mutable_$name$() gives us a string reference and sets the has bit
    // for $name$ (in proto2).  We may get here when the string field is inlined
    // but the string's contents have not been changed by the user, so we cannot
    // make an assertion about the contents of the string and could never make
    // an assertion about the string instance.
    //
    // For non-inlined strings, we distinguish from non-default by comparing
    // instances, rather than contents.
    printer->Print(variables_,
      "GOOGLE_DCHECK(!$name$_.IsDefault($default_variable$));\n");
  }

  if (SupportsArenas(descriptor_)) {
    if (descriptor_->default_value_string().empty()) {
      if (must_be_present) {
        printer->Print(variables_,
          "$name$_.ClearNonDefaultToEmpty();\n");
      } else {
        printer->Print(variables_,
          "$name$_.ClearToEmpty($default_variable$, GetArenaNoVirtual());\n");
      }
    } else {
      // Clear to a non-empty default is more involved, as we try to use the
      // Arena if one is present and may need to reallocate the string.
      printer->Print(variables_,
        "$name$_.ClearToDefault($default_variable$, GetArenaNoVirtual());\n");
    }
  } else if (must_be_present) {
    // When Arenas are disabled and field presence has been checked, we can
    // safely treat the ArenaStringPtr as a string*.
    if (descriptor_->default_value_string().empty()) {
      printer->Print(variables_, "$name$_.ClearNonDefaultToEmptyNoArena();\n");
    } else {
      printer->Print(
          variables_,
          "$name$_.UnsafeMutablePointer()->assign(*$default_variable$);\n");
    }
  } else {
    if (descriptor_->default_value_string().empty()) {
      printer->Print(variables_,
        "$name$_.ClearToEmptyNoArena($default_variable$);\n");
    } else {
      printer->Print(variables_,
        "$name$_.ClearToDefaultNoArena($default_variable$);\n");
    }
  }
}

void StringFieldGenerator::
GenerateMergingCode(io::Printer* printer) const {
  if (SupportsArenas(descriptor_) || descriptor_->containing_oneof() != NULL) {
    // TODO(gpike): improve this
    printer->Print(variables_, "set_$name$(from.$name$());\n");
  } else {
    printer->Print(variables_,
      "$set_hasbit$\n"
      "$name$_.AssignWithDefault($default_variable$, from.$name$_);\n");
  }
}

void StringFieldGenerator::
GenerateSwappingCode(io::Printer* printer) const {
  if (inlined_) {
    printer->Print(
        variables_,
        "$name$_.Swap(&other->$name$_);\n");
  } else {
    printer->Print(
        variables_,
        "$name$_.Swap(&other->$name$_, $default_variable$,\n"
        "  GetArenaNoVirtual());\n");
  }
}

void StringFieldGenerator::
GenerateConstructorCode(io::Printer* printer) const {
  // TODO(ckennelly): Construct non-empty strings as part of the initializer
  // list.
  if (inlined_ && descriptor_->default_value_string().empty()) {
    // Automatic initialization will construct the string.
    return;
  }

  printer->Print(variables_,
                 "$name$_.UnsafeSetDefault($default_variable$);\n");
}

void StringFieldGenerator::
GenerateCopyConstructorCode(io::Printer* printer) const {
  GenerateConstructorCode(printer);

  if (HasFieldPresence(descriptor_->file())) {
    printer->Print(variables_,
        "if (from.has_$name$()) {\n");
  } else {
    printer->Print(variables_,
        "if (from.$name$().size() > 0) {\n");
  }

  printer->Indent();

  if (SupportsArenas(descriptor_) || descriptor_->containing_oneof() != NULL) {
    // TODO(gpike): improve this
    printer->Print(variables_,
      "$name$_.Set$lite$($default_variable$, from.$name$(),\n"
      "  GetArenaNoVirtual());\n");
  } else {
    printer->Print(variables_,
      "$name$_.AssignWithDefault($default_variable$, from.$name$_);\n");
  }

  printer->Outdent();
  printer->Print("}\n");
}

void StringFieldGenerator::
GenerateDestructorCode(io::Printer* printer) const {
  if (inlined_) {
    // The destructor is automatically invoked.
    return;
  }

  printer->Print(variables_, "$name$_.DestroyNoArena($default_variable$);\n");
}

bool StringFieldGenerator::GenerateArenaDestructorCode(
    io::Printer* printer) const {
  if (!inlined_) {
    return false;
  }

  printer->Print(variables_,
                 "_this->$name$_.DestroyNoArena($default_variable$);\n");
  return true;
}

void StringFieldGenerator::
GenerateDefaultInstanceAllocator(io::Printer* printer) const {
  if (!descriptor_->default_value_string().empty()) {
    printer->Print(
        variables_,
        "$ns$::$classname$::$default_variable_name$.DefaultConstruct();\n"
        "*$ns$::$classname$::$default_variable_name$.get_mutable() = "
        "::std::string($default$, $default_length$);\n"
        "::google::protobuf::internal::OnShutdownDestroyString(\n"
        "    $ns$::$classname$::$default_variable_name$.get_mutable());\n"
    );
  }
}

void StringFieldGenerator::
GenerateMergeFromCodedStream(io::Printer* printer) const {
  printer->Print(variables_,
    "DO_(::google::protobuf::internal::WireFormatLite::Read$declared_type$(\n"
    "      input, this->mutable_$name$()));\n");

  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, true, variables_,
        "this->$name$().data(), static_cast<int>(this->$name$().length()),\n",
        printer);
  }
}

bool StringFieldGenerator::
MergeFromCodedStreamNeedsArena() const {
  return false;
}

void StringFieldGenerator::
GenerateSerializeWithCachedSizes(io::Printer* printer) const {
  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, false, variables_,
        "this->$name$().data(), static_cast<int>(this->$name$().length()),\n",
        printer);
  }
  printer->Print(variables_,
    "::google::protobuf::internal::WireFormatLite::Write$declared_type$MaybeAliased(\n"
    "  $number$, this->$name$(), output);\n");
}

void StringFieldGenerator::
GenerateSerializeWithCachedSizesToArray(io::Printer* printer) const {
  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, false, variables_,
        "this->$name$().data(), static_cast<int>(this->$name$().length()),\n",
        printer);
  }
  printer->Print(variables_,
    "target =\n"
    "  ::google::protobuf::internal::WireFormatLite::Write$declared_type$ToArray(\n"
    "    $number$, this->$name$(), target);\n");
}

void StringFieldGenerator::
GenerateByteSize(io::Printer* printer) const {
  printer->Print(variables_,
    "total_size += $tag_size$ +\n"
    "  ::google::protobuf::internal::WireFormatLite::$declared_type$Size(\n"
    "    this->$name$());\n");
}

uint32 StringFieldGenerator::CalculateFieldTag() const {
  return inlined_ ? 1 : 0;
}

// ===================================================================

StringOneofFieldGenerator::StringOneofFieldGenerator(
    const FieldDescriptor* descriptor, const Options& options)
    : StringFieldGenerator(descriptor, options) {
  inlined_ = false;

  SetCommonOneofFieldVariables(descriptor, &variables_);
}

StringOneofFieldGenerator::~StringOneofFieldGenerator() {}

void StringOneofFieldGenerator::
GenerateInlineAccessorDefinitions(io::Printer* printer) const {
  if (SupportsArenas(descriptor_)) {
    printer->Print(
        variables_,
        "inline const ::std::string& $classname$::$name$() const {\n"
        "  // @@protoc_insertion_point(field_get:$full_name$)\n"
        "  if (has_$name$()) {\n"
        "    return $field_member$.Get();\n"
        "  }\n"
        "  return *$default_variable$;\n"
        "}\n"
        "inline void $classname$::set_$name$(const ::std::string& value) {\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.Set$lite$($default_variable$, value,\n"
        "      GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "}\n"
        "#if LANG_CXX11\n"
        "inline void $classname$::set_$name$(::std::string&& value) {\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.Set$lite$(\n"
        "    $default_variable$, ::std::move(value), GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_rvalue:$full_name$)\n"
        "}\n"
        "#endif\n"
        "inline void $classname$::set_$name$(const char* value) {\n"
        "  $null_check$"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.Set$lite$($default_variable$,\n"
        "      $string_piece$(value), GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_char:$full_name$)\n"
        "}\n"
        "inline "
        "void $classname$::set_$name$(const $pointer_type$* value,\n"
        "                             size_t size) {\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.Set$lite$(\n"
        "      $default_variable$, $string_piece$(\n"
        "      reinterpret_cast<const char*>(value), size),\n"
        "      GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_set_pointer:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::mutable_$name$() {\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  return $field_member$.Mutable($default_variable$,\n"
        "      GetArenaNoVirtual());\n"
        "  // @@protoc_insertion_point(field_mutable:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::$release_name$() {\n"
        "  // @@protoc_insertion_point(field_release:$full_name$)\n"
        "  if (has_$name$()) {\n"
        "    clear_has_$oneof_name$();\n"
        "    return $field_member$.Release($default_variable$,\n"
        "        GetArenaNoVirtual());\n"
        "  } else {\n"
        "    return NULL;\n"
        "  }\n"
        "}\n"
        "inline void $classname$::set_allocated_$name$(::std::string* $name$) {\n"
        "  if (!has_$name$()) {\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  clear_$oneof_name$();\n"
        "  if ($name$ != NULL) {\n"
        "    set_has_$name$();\n"
        "    $field_member$.SetAllocated($default_variable$, $name$,\n"
        "        GetArenaNoVirtual());\n"
        "  }\n"
        "  // @@protoc_insertion_point(field_set_allocated:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::unsafe_arena_release_$name$() {\n"
        "  // "
        "@@protoc_insertion_point(field_unsafe_arena_release:$full_name$)\n"
        "  GOOGLE_DCHECK(GetArenaNoVirtual() != NULL);\n"
        "  if (has_$name$()) {\n"
        "    clear_has_$oneof_name$();\n"
        "    return $field_member$.UnsafeArenaRelease(\n"
        "        $default_variable$, GetArenaNoVirtual());\n"
        "  } else {\n"
        "    return NULL;\n"
        "  }\n"
        "}\n"
        "inline void $classname$::unsafe_arena_set_allocated_$name$("
        "::std::string* $name$) {\n"
        "  GOOGLE_DCHECK(GetArenaNoVirtual() != NULL);\n"
        "  if (!has_$name$()) {\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  clear_$oneof_name$();\n"
        "  if ($name$) {\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeArenaSetAllocated($default_variable$, "
        "$name$, GetArenaNoVirtual());\n"
        "  }\n"
        "  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:"
        "$full_name$)\n"
        "}\n");
  } else {
    // No-arena case.
    printer->Print(
        variables_,
        "inline const ::std::string& $classname$::$name$() const {\n"
        "  // @@protoc_insertion_point(field_get:$full_name$)\n"
        "  if (has_$name$()) {\n"
        "    return $field_member$.GetNoArena();\n"
        "  }\n"
        "  return *$default_variable$;\n"
        "}\n"
        "inline void $classname$::set_$name$(const ::std::string& value) {\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.SetNoArena($default_variable$, value);\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "}\n"
        "#if LANG_CXX11\n"
        "inline void $classname$::set_$name$(::std::string&& value) {\n"
        "  // @@protoc_insertion_point(field_set:$full_name$)\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.SetNoArena($default_variable$, ::std::move(value));\n"
        "  // @@protoc_insertion_point(field_set_rvalue:$full_name$)\n"
        "}\n"
        "#endif\n"
        "inline void $classname$::set_$name$(const char* value) {\n"
        "  $null_check$"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.SetNoArena($default_variable$,\n"
        "      $string_piece$(value));\n"
        "  // @@protoc_insertion_point(field_set_char:$full_name$)\n"
        "}\n"
        "inline "
        "void $classname$::set_$name$(const $pointer_type$* value, size_t "
        "size) {\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  $field_member$.SetNoArena($default_variable$, $string_piece$(\n"
        "      reinterpret_cast<const char*>(value), size));\n"
        "  // @@protoc_insertion_point(field_set_pointer:$full_name$)\n"
        "}\n"
        "inline ::std::string* $classname$::mutable_$name$() {\n"
        "  if (!has_$name$()) {\n"
        "    clear_$oneof_name$();\n"
        "    set_has_$name$();\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  // @@protoc_insertion_point(field_mutable:$full_name$)\n"
        "  return $field_member$.MutableNoArena($default_variable$);\n"
        "}\n"
        "inline ::std::string* $classname$::$release_name$() {\n"
        "  // @@protoc_insertion_point(field_release:$full_name$)\n"
        "  if (has_$name$()) {\n"
        "    clear_has_$oneof_name$();\n"
        "    return $field_member$.ReleaseNoArena($default_variable$);\n"
        "  } else {\n"
        "    return NULL;\n"
        "  }\n"
        "}\n"
        "inline void $classname$::set_allocated_$name$(::std::string* $name$) {\n"
        "  if (!has_$name$()) {\n"
        "    $field_member$.UnsafeSetDefault($default_variable$);\n"
        "  }\n"
        "  clear_$oneof_name$();\n"
        "  if ($name$ != NULL) {\n"
        "    set_has_$name$();\n"
        "    $field_member$.SetAllocatedNoArena($default_variable$, $name$);\n"
        "  }\n"
        "  // @@protoc_insertion_point(field_set_allocated:$full_name$)\n"
        "}\n");
  }
}

void StringOneofFieldGenerator::
GenerateClearingCode(io::Printer* printer) const {
  if (SupportsArenas(descriptor_)) {
    printer->Print(variables_,
      "$field_member$.Destroy($default_variable$,\n"
      "    GetArenaNoVirtual());\n");
  } else {
    printer->Print(variables_,
      "$field_member$.DestroyNoArena($default_variable$);\n");
  }
}

void StringOneofFieldGenerator::
GenerateMessageClearingCode(io::Printer* printer) const {
  return GenerateClearingCode(printer);
}

void StringOneofFieldGenerator::
GenerateSwappingCode(io::Printer* printer) const {
  // Don't print any swapping code. Swapping the union will swap this field.
}

void StringOneofFieldGenerator::
GenerateConstructorCode(io::Printer* printer) const {
  printer->Print(
      variables_,
      "$ns$::_$classname$_default_instance_.$name$_.UnsafeSetDefault(\n"
      "    $default_variable$);\n");
}

void StringOneofFieldGenerator::
GenerateDestructorCode(io::Printer* printer) const {
  printer->Print(variables_,
    "if (has_$name$()) {\n"
    "  $field_member$.DestroyNoArena($default_variable$);\n"
    "}\n");
}

void StringOneofFieldGenerator::
GenerateMergeFromCodedStream(io::Printer* printer) const {
    printer->Print(variables_,
      "DO_(::google::protobuf::internal::WireFormatLite::Read$declared_type$(\n"
      "      input, this->mutable_$name$()));\n");

  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, true, variables_,
        "this->$name$().data(), static_cast<int>(this->$name$().length()),\n",
        printer);
  }
}


// ===================================================================

RepeatedStringFieldGenerator::RepeatedStringFieldGenerator(
    const FieldDescriptor* descriptor, const Options& options)
    : FieldGenerator(options), descriptor_(descriptor) {
  SetStringVariables(descriptor, &variables_, options);
}

RepeatedStringFieldGenerator::~RepeatedStringFieldGenerator() {}

void RepeatedStringFieldGenerator::
GeneratePrivateMembers(io::Printer* printer) const {
  printer->Print(variables_,
    "::google::protobuf::RepeatedPtrField< ::std::string> $name$_;\n");
}

void RepeatedStringFieldGenerator::
GenerateAccessorDeclarations(io::Printer* printer) const {
  // See comment above about unknown ctypes.
  bool unknown_ctype =
      descriptor_->options().ctype() != EffectiveStringCType(descriptor_);

  if (unknown_ctype) {
    printer->Outdent();
    printer->Print(
      " private:\n"
      "  // Hidden due to unknown ctype option.\n");
    printer->Indent();
  }

  printer->Print(variables_,
                 "$deprecated_attr$const ::std::string& $name$(int index) const;\n");
  printer->Annotate("name", descriptor_);
  printer->Print(
      variables_,
      "$deprecated_attr$::std::string* ${$mutable_$name$$}$(int index);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$void ${$set_$name$$}$(int index, const "
                 "::std::string& value);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(
      variables_,
      "#if LANG_CXX11\n"
      "$deprecated_attr$void ${$set_$name$$}$(int index, ::std::string&& value);\n"
      "#endif\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$void ${$set_$name$$}$(int index, const "
                 "char* value);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 ""
                 "$deprecated_attr$void ${$set_$name$$}$("
                 "int index, const $pointer_type$* value, size_t size);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$::std::string* ${$add_$name$$}$();\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(
      variables_,
      "$deprecated_attr$void ${$add_$name$$}$(const ::std::string& value);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "#if LANG_CXX11\n"
                 "$deprecated_attr$void ${$add_$name$$}$(::std::string&& value);\n"
                 "#endif\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(
      variables_,
      "$deprecated_attr$void ${$add_$name$$}$(const char* value);\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$void ${$add_$name$$}$(const $pointer_type$* "
                 "value, size_t size)"
                 ";\n");
  printer->Annotate("{", "}", descriptor_);
  printer->Print(
      variables_,
      "$deprecated_attr$const ::google::protobuf::RepeatedPtrField< ::std::string>& $name$() "
      "const;\n");
  printer->Annotate("name", descriptor_);
  printer->Print(variables_,
                 "$deprecated_attr$::google::protobuf::RepeatedPtrField< ::std::string>* "
                 "${$mutable_$name$$}$()"
                 ";\n");
  printer->Annotate("{", "}", descriptor_);

  if (unknown_ctype) {
    printer->Outdent();
    printer->Print(" public:\n");
    printer->Indent();
  }
}

void RepeatedStringFieldGenerator::
GenerateInlineAccessorDefinitions(io::Printer* printer) const {
  if (options_.safe_boundary_check) {
    printer->Print(variables_,
      "inline const ::std::string& $classname$::$name$(int index) const {\n"
      "  // @@protoc_insertion_point(field_get:$full_name$)\n"
      "  return $name$_.InternalCheckedGet(\n"
      "      index, ::google::protobuf::internal::GetEmptyStringAlreadyInited());\n"
      "}\n");
  } else {
    printer->Print(variables_,
      "inline const ::std::string& $classname$::$name$(int index) const {\n"
      "  // @@protoc_insertion_point(field_get:$full_name$)\n"
      "  return $name$_.Get(index);\n"
      "}\n");
  }
  printer->Print(variables_,
    "inline ::std::string* $classname$::mutable_$name$(int index) {\n"
    "  // @@protoc_insertion_point(field_mutable:$full_name$)\n"
    "  return $name$_.Mutable(index);\n"
    "}\n"
    "inline void $classname$::set_$name$(int index, const ::std::string& value) {\n"
    "  // @@protoc_insertion_point(field_set:$full_name$)\n"
    "  $name$_.Mutable(index)->assign(value);\n"
    "}\n"
    "#if LANG_CXX11\n"
    "inline void $classname$::set_$name$(int index, ::std::string&& value) {\n"
    "  // @@protoc_insertion_point(field_set:$full_name$)\n"
    "  $name$_.Mutable(index)->assign(std::move(value));\n"
    "}\n"
    "#endif\n"
    "inline void $classname$::set_$name$(int index, const char* value) {\n"
    "  $null_check$"
    "  $name$_.Mutable(index)->assign(value);\n"
    "  // @@protoc_insertion_point(field_set_char:$full_name$)\n"
    "}\n"
    "inline void "
    "$classname$::set_$name$"
    "(int index, const $pointer_type$* value, size_t size) {\n"
    "  $name$_.Mutable(index)->assign(\n"
    "    reinterpret_cast<const char*>(value), size);\n"
    "  // @@protoc_insertion_point(field_set_pointer:$full_name$)\n"
    "}\n"
    "inline ::std::string* $classname$::add_$name$() {\n"
    "  // @@protoc_insertion_point(field_add_mutable:$full_name$)\n"
    "  return $name$_.Add();\n"
    "}\n"
    "inline void $classname$::add_$name$(const ::std::string& value) {\n"
    "  $name$_.Add()->assign(value);\n"
    "  // @@protoc_insertion_point(field_add:$full_name$)\n"
    "}\n"
    "#if LANG_CXX11\n"
    "inline void $classname$::add_$name$(::std::string&& value) {\n"
    "  $name$_.Add(std::move(value));\n"
    "  // @@protoc_insertion_point(field_add:$full_name$)\n"
    "}\n"
    "#endif\n"
    "inline void $classname$::add_$name$(const char* value) {\n"
    "  $null_check$"
    "  $name$_.Add()->assign(value);\n"
    "  // @@protoc_insertion_point(field_add_char:$full_name$)\n"
    "}\n"
    "inline void "
    "$classname$::add_$name$(const $pointer_type$* value, size_t size) {\n"
    "  $name$_.Add()->assign(reinterpret_cast<const char*>(value), size);\n"
    "  // @@protoc_insertion_point(field_add_pointer:$full_name$)\n"
    "}\n"
    "inline const ::google::protobuf::RepeatedPtrField< ::std::string>&\n"
    "$classname$::$name$() const {\n"
    "  // @@protoc_insertion_point(field_list:$full_name$)\n"
    "  return $name$_;\n"
    "}\n"
    "inline ::google::protobuf::RepeatedPtrField< ::std::string>*\n"
    "$classname$::mutable_$name$() {\n"
    "  // @@protoc_insertion_point(field_mutable_list:$full_name$)\n"
    "  return &$name$_;\n"
    "}\n");
}

void RepeatedStringFieldGenerator::
GenerateClearingCode(io::Printer* printer) const {
  printer->Print(variables_, "$name$_.Clear();\n");
}

void RepeatedStringFieldGenerator::
GenerateMergingCode(io::Printer* printer) const {
  printer->Print(variables_, "$name$_.MergeFrom(from.$name$_);\n");
}

void RepeatedStringFieldGenerator::
GenerateSwappingCode(io::Printer* printer) const {
  printer->Print(variables_,
                 "$name$_.InternalSwap(CastToBase(&other->$name$_));\n");
}

void RepeatedStringFieldGenerator::
GenerateConstructorCode(io::Printer* printer) const {
  // Not needed for repeated fields.
}

void RepeatedStringFieldGenerator::
GenerateCopyConstructorCode(io::Printer* printer) const {
  printer->Print(variables_, "$name$_.CopyFrom(from.$name$_);");
}

void RepeatedStringFieldGenerator::
GenerateMergeFromCodedStream(io::Printer* printer) const {
  printer->Print(variables_,
    "DO_(::google::protobuf::internal::WireFormatLite::Read$declared_type$(\n"
    "      input, this->add_$name$()));\n");
  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, true, variables_,
        "this->$name$(this->$name$_size() - 1).data(),\n"
        "static_cast<int>(this->$name$(this->$name$_size() - 1).length()),\n",
        printer);
  }
}

void RepeatedStringFieldGenerator::
GenerateSerializeWithCachedSizes(io::Printer* printer) const {
  printer->Print(variables_,
        "for (int i = 0, n = this->$name$_size(); i < n; i++) {\n");
  printer->Indent();
  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, false, variables_,
        "this->$name$(i).data(), static_cast<int>(this->$name$(i).length()),\n",
        printer);
  }
  printer->Outdent();
  printer->Print(variables_,
    "  ::google::protobuf::internal::WireFormatLite::Write$declared_type$(\n"
    "    $number$, this->$name$(i), output);\n"
    "}\n");
}

void RepeatedStringFieldGenerator::
GenerateSerializeWithCachedSizesToArray(io::Printer* printer) const {
  printer->Print(variables_,
    "for (int i = 0, n = this->$name$_size(); i < n; i++) {\n");
  printer->Indent();
  if (descriptor_->type() == FieldDescriptor::TYPE_STRING) {
    GenerateUtf8CheckCodeForString(
        descriptor_, options_, false, variables_,
        "this->$name$(i).data(), static_cast<int>(this->$name$(i).length()),\n",
        printer);
  }
  printer->Outdent();
  printer->Print(variables_,
    "  target = ::google::protobuf::internal::WireFormatLite::\n"
    "    Write$declared_type$ToArray($number$, this->$name$(i), target);\n"
    "}\n");
}

void RepeatedStringFieldGenerator::
GenerateByteSize(io::Printer* printer) const {
  printer->Print(variables_,
    "total_size += $tag_size$ *\n"
    "    ::google::protobuf::internal::FromIntSize(this->$name$_size());\n"
    "for (int i = 0, n = this->$name$_size(); i < n; i++) {\n"
    "  total_size += ::google::protobuf::internal::WireFormatLite::$declared_type$Size(\n"
    "    this->$name$(i));\n"
    "}\n");
}

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

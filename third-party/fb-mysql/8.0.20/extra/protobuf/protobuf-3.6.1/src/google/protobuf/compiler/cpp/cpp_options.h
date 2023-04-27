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

// Author: rennie@google.com (Jeffrey Rennie)

#ifndef GOOGLE_PROTOBUF_COMPILER_CPP_OPTIONS_H__
#define GOOGLE_PROTOBUF_COMPILER_CPP_OPTIONS_H__

#include <string>

#include <google/protobuf/stubs/common.h>
namespace google {
namespace protobuf {
namespace compiler {
class AccessInfoMap;

namespace cpp {

// Generator options (see generator.cc for a description of each):
struct Options {
  Options()
      : safe_boundary_check(false),
        proto_h(false),
        transitive_pb_h(true),
        annotate_headers(false),
        enforce_lite(false),
        table_driven_parsing(false),
        table_driven_serialization(false),
        lite_implicit_weak_fields(false),
        bootstrap(false),
        num_cc_files(0),
        access_info_map(NULL) {}

  string dllexport_decl;
  bool safe_boundary_check;
  bool proto_h;
  bool transitive_pb_h;
  bool annotate_headers;
  bool enforce_lite;
  bool table_driven_parsing;
  bool table_driven_serialization;
  bool lite_implicit_weak_fields;
  bool bootstrap;
  int num_cc_files;
  string annotation_pragma_name;
  string annotation_guard_name;
  const AccessInfoMap* access_info_map;
};

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf


}  // namespace google
#endif  // GOOGLE_PROTOBUF_COMPILER_CPP_OPTIONS_H__

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <sstream>
#include <string>

#include <thrift/compiler/ast/t_program.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace java {

/**
 * Mangles an identifier for use in generated Java. Ported from
 * TemplateContextGenerator.java::mangleJavaName
 * from the java implementation of the swift generator.
 * http://tinyurl.com/z7vocup
 */
std::string mangle_java_name(const std::string& ref, bool capitalize);

/**
 * Mangles an identifier for use in generated Java as a constant.
 * Ported from TemplateContextGenerator.java::mangleJavaConstantName
 * from the java implementation of the swift generator.
 * http://tinyurl.com/z7vocup
 */
std::string mangle_java_constant_name(const std::string& ref);

/**
 * Perform the following:
 * (1) Strings in Java are always encoded in UTF-16 by default. (reference:
 * https://docs.oracle.com/javase/specs/jls/se8/html/jls-3.html). Therefore
 * Java do not support \x escape. We need to escape thrift string in a proper
 * way.
 * (2) escape `"` to `\"`
 */
std::string quote_java_string(const std::string& unescaped);

/**
 * Converts a java package string to the path containing the source for
 * that package. Example: "foo.bar.baz" -> "foo/bar/baz"
 */
std::string package_to_path(std::string package);

} // namespace java
} // namespace compiler
} // namespace thrift
} // namespace apache

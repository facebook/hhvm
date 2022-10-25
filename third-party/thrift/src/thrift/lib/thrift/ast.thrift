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

include "thrift/lib/thrift/schema.thrift"

// A lightweight source range that can be resolved into the file name,
// line and column when the schema is produced from an IDL file.
struct SourceRange {
  1: id.ProgramId programId;
  2: i32 beginOffset;
  3: i32 endOffset;
}

/**
 * Information about a thrift source file.
 */
struct SourceInfo {
  // A source file name. It can include directory components and/or be a
  // virtual file name that doesn't have a correspondent entry in the system's
  // directory structure.
  1: string fileName;

  // Line offsets in code units pointing to the beginning of each line.
  // For example:
  //   [0, 10, 15]
  // means that the first line consists of 10 code units including newline and
  // the second line is 5 code units and doesn't have a newlne.
  2: list<i32> lineOffsets;
}

/**
 * Per-language include statements.
 */
union LanguageInclude {
  1: string cppInclude;
  2: string hsInclude;
}

// A thrift schema that corresponds to one or more thrift files.
struct AST {
  /**
   * The content of the program.
   * The first program in the program list must be the main file,
   * with (recursively) included programs listed after it.
   */
  1: schema.Schema schema;

  /**
   * Information about the files holding the thrift definitions.
   */
  2: map<id.ProgramId, SourceInfo> sources;

  /**
   * Information about where each definition is present in the source files.
   */
  3: map<id.DefinitionId, SourceRange> sourceRanges;

  /**
   * Additional per-language includes not represented in the schema.
   */
  4: map<id.ProgramId, list<LanguageInclude>> languageIncludes;
}

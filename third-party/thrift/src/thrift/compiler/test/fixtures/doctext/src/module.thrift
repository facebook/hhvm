/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

/** Constant foo */
const string FOO = "foo";

/// Multi-
/// line
/// slash comment
const i32 BAR = 123;

/// BIFF has your mail
const i32 BIFF = 0;

// Banners are not doc comments
//////////////////////////////////////////
///////////// SHOUTY /////////////////////
//////////////////////////////////////////
const i32 SHOUTY = 11;

/** Cool new name for string */
typedef string lanyard

/** Secret name */
typedef i32 number (rust.newtype)

/**
 * DefinitionList doctext.
 */
struct A {
  /**
   * Field doctext.
   */
  1: i32 useless_field;
}

/** This enum is great */
enum B {
  /**
   * EnumDef doctext.
   */
  HELLO = 0, // This comment will not be added to the generated code
}

/** Union U */
union U {
  /** i32 field i */
  1: i32 i;
  /** string s */
  2: string s;
}

/** Something amiss */
exception Bang {
  /** All explosions can be explained away */
  1: string message;
}

/** Detailed overview of service */
service C {
  /**
   * Function doctext.
   */
  void f();

  /** Streaming function */
  stream<number> numbers();

  // test empty doc comment
  ///
  string thing(1: i32 a, 2: string b, 3: set<i32> c) throws (1: Bang bang);
}

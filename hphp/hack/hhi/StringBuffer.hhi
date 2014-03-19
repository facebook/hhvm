<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * Type information for builtin class StringBuffer
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */
class StringBuffer {

  public function append(?mixed $value): void;

  public function detach(): string;
}

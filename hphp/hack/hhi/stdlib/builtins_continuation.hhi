<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function hphp_create_continuation($clsname, $funcname, $origFuncName, $args = null) { }
class DummyContinuation {
  public function __construct() { }
  public function current() { }
  public function key() { }
  public function next() { }
  public function rewind() { }
  public function valid() { }
}

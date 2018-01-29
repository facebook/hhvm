<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator___construct($obj, $iterator, $mode, $flags) { }
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator_getinneriterator($obj) { }
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator_current($obj) { }
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator_key($obj) { }
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator_next($obj) { }
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator_rewind($obj) { }
<<__PHPStdLib>>
function hphp_recursiveiteratoriterator_valid($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator___construct($obj, $path) { }
<<__PHPStdLib>>
function hphp_directoryiterator_key($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator_next($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator_rewind($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator_seek($obj, $position) { }
<<__PHPStdLib>>
function hphp_directoryiterator_current($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator___tostring($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator_valid($obj) { }
<<__PHPStdLib>>
function hphp_directoryiterator_isdot($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator___construct($obj, $path, $flags) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_key($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_next($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_rewind($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_seek($obj, $position) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_current($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator___tostring($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_valid($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_haschildren($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_getchildren($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_getsubpath($obj) { }
<<__PHPStdLib>>
function hphp_recursivedirectoryiterator_getsubpathname($obj) { }
class MutableArrayIterator {
  public function __construct(&$array) { }
  public function currentRef() { }
  public function current() { }
  public function key() { }
  public function next() { }
  public function valid() { }
}

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
function bzclose($bz) { }
<<__PHPStdLib>>
function bzopen($filename, $mode) { }
<<__PHPStdLib>>
function bzread($bz, $length = 1024) { }
<<__PHPStdLib>>
function bzwrite($bz, $data, $length = 0) { }
<<__PHPStdLib>>
function bzflush($bz) { }
<<__PHPStdLib>>
function bzerrstr($bz) { }
<<__PHPStdLib>>
function bzerror($bz) { }
<<__PHPStdLib>>
function bzerrno($bz) { }
<<__PHPStdLib>>
function bzcompress($source, $blocksize = 4, $workfactor = 0) { }
<<__PHPStdLib>>
function bzdecompress($source, $small = 0) { }

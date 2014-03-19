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
function bzclose($bz) { }
function bzopen($filename, $mode) { }
function bzread($bz, $length = 1024) { }
function bzwrite($bz, $data, $length = 0) { }
function bzflush($bz) { }
function bzerrstr($bz) { }
function bzerror($bz) { }
function bzerrno($bz) { }
function bzcompress($source, $blocksize = 4, $workfactor = 0) { }
function bzdecompress($source, $small = 0) { }

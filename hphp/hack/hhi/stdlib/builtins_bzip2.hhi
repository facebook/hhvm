<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function bzclose(resource $bz): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzopen(
  HH\FIXME\MISSING_PARAM_TYPE $filename,
  string $mode,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzread(resource $bz, int $length = 1024): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzwrite(
  resource $bz,
  string $data,
  int $length = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzflush(resource $bz): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzerrstr(resource $bz): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzerror(resource $bz): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzerrno(resource $bz): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzcompress(
  string $source,
  int $blocksize = 4,
  int $workfactor = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bzdecompress(
  string $source,
  int $small = 0,
)[]: HH\FIXME\MISSING_RETURN_TYPE;

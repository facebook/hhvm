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
function bzclose(resource $bz);
<<__PHPStdLib>>
function bzopen($filename, string $mode);
<<__PHPStdLib>>
function bzread(resource $bz, int $length = 1024);
<<__PHPStdLib>>
function bzwrite(resource $bz, string $data, int $length = 0);
<<__PHPStdLib>>
function bzflush(resource $bz);
<<__PHPStdLib>>
function bzerrstr(resource $bz);
<<__PHPStdLib>>
function bzerror(resource $bz);
<<__PHPStdLib>>
function bzerrno(resource $bz);
<<__PHPStdLib, __Rx>>
function bzcompress(string $source, int $blocksize = 4, int $workfactor = 0);
<<__PHPStdLib, __Rx>>
function bzdecompress(string $source, int $small = 0);

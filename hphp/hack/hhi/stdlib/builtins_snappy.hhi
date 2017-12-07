<?hh // decl /* -*- php -*- */
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
function sncompress($data);
<<__PHPStdLib>>
function snuncompress($data);
<<__PHPStdLib>>
function snappy_compress(string $data): mixed;
function snappy_uncompress(string $data): mixed;

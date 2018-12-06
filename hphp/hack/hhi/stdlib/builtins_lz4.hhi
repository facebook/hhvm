<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib, __Rx>>
function lz4_compress(string $uncompressed, bool $high = false): mixed;
<<__PHPStdLib, __Rx>>
function lz4_hccompress(string $uncompressed): mixed;
<<__PHPStdLib, __Rx>>
function lz4_uncompress(string $compressed): mixed;

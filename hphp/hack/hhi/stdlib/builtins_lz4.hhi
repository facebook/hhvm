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

function lz4_compress(string $uncompressed, bool $high = false): mixed;
function lz4_hccompress(string $uncompressed): mixed;
function lz4_uncompress(string $compressed): mixed;

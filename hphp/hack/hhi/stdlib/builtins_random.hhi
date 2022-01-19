<?hh /* -*- php -*- */
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function random_bytes(int $length)[leak_safe]: string;

<<__PHPStdLib>>
function random_int(int $min, int $max)[leak_safe]: int;

<?hh
// Copyright (c) 2022, Meta, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// isset, and unset only look like functions to static analysis, but
// in fact act as special expression subtypes within the runtime

/**
 * Determines whether or not a variable is "set."
 */
function isset(mixed $x)[]: bool;

/**
 * Used to remove keys from arrays:
 *
 * ```
 * $d = dict[42 => true];
 * unset($d[42]);
 * var_dump($d); // prints an empty dict
 * ```
 *
 * Also can be used to unset object properties and variables, but this behavior
 * is disallowed by Hack.
 */
function unset(mixed $x)[]: void;

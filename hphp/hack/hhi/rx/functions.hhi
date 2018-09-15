<?hh // decl   /* -*- php -*- */
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined functions
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH\Rx;

// freeze is a special function for mutability
function freeze($x) : void;

// mutable is a special function to indicate ownership
// transfer for fresh mutable values
<<__Rx>>
function mutable<T>(T $x): T;

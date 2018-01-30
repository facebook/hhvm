<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * This file provides type information for some of PHP's predefined functions
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */
<<__PHPStdLib>>
function array_key_exists<Tk, Tv>(mixed $key, ?KeyedContainer<Tk, Tv> $search): bool;

<<__PHPStdLib>>
function array_sum/*<T>*/(/*Container<T>*/ $input)/*: num*/;
<<__PHPStdLib>>
function array_product/*<T>*/(/*Container<T>*/ $input)/*: num*/;

<<__PHPStdLib>>
function sort<Tv>(Container<Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
<<__PHPStdLib>>
function rsort<Tv>(Container<Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
<<__PHPStdLib>>
function asort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
<<__PHPStdLib>>
function arsort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
<<__PHPStdLib>>
function ksort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR): bool;
<<__PHPStdLib>>
function krsort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR): bool;
// $c is a callable of type (function(Tv,Tv): bool)
<<__PHPStdLib>>
function usort<Tv>(Container<Tv> &$arg, mixed $c): bool;
// $c is a callable of type (function(Tv,Tv): bool)
<<__PHPStdLib>>
function uasort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, mixed $c): bool;
// $c is a callable of type (function(Tk,Tk): bool)
<<__PHPStdLib>>
function uksort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, mixed $c): bool;

/**
 * Creates a `dict` from a `KeyedTraversable`, preserving keys and order.
 */
<<__Rx>>
function dict<Tk, Tv>(KeyedTraversable<Tk, Tv> $arr): dict<Tk, Tv>;
/**
 * Creates a `vec` from a `Traversable`, preserving order. Keys are not
 * preserved.
 */
<<__Rx>>
function vec<Tv>(Traversable<Tv> $arr): vec<Tv>;
/**
 * Create a `keyset` from a `Traversable` of strings or ints, preserving order.
 * Keys are not preserved.
 */
<<__Rx>>
function keyset<Tv as arraykey>(Traversable<Tv> $arr): keyset<Tv>;

<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
<<file:__EnableUnstableFeatures('readonly')>>

/**
 * This file provides type information for some of PHP's predefined functions
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace {

<<__PHPStdLib>>
function array_key_exists(mixed $key, readonly ?KeyedContainer<arraykey, mixed> $search)[]: bool;

<<__PHPStdLib>>
function array_sum/*<T>*/(readonly /*Container<T>*/ $input)[]/*: num*/;
<<__PHPStdLib>>
function array_product/*<T>*/(readonly /*Container<T>*/ $input)[]/*: num*/;

<<__PHPStdLib>>
function sort<T as Container<mixed>>(

  inout T $arg,
  int $sort_flags = SORT_REGULAR,
)[]: bool;
<<__PHPStdLib>>
function rsort<T as Container<mixed>>(

  inout T $arg,
  int $sort_flags = SORT_REGULAR,
)[]: bool;
<<__PHPStdLib>>
function asort<Tk as arraykey, Tv>(

  inout KeyedContainer<Tk, Tv> $arg,
  int $sort_flags = SORT_REGULAR,
)[]: bool;
<<__PHPStdLib>>
function arsort<Tk as arraykey, Tv>(

  inout KeyedContainer<Tk, Tv> $arg,
  int $sort_flags = SORT_REGULAR,
)[]: bool;
<<__PHPStdLib>>
function ksort<T as KeyedContainer<arraykey, mixed>>(

  inout T $arg,
  int $sort_flags = SORT_REGULAR,
)[]: bool;
<<__PHPStdLib>>
function krsort<Tk as arraykey, Tv>(

  inout KeyedContainer<Tk, Tv> $arg,
  int $sort_flags = SORT_REGULAR,
)[]: bool;
<<__PHPStdLib>>
function usort<Tv, T as Container<Tv>>(

  inout T $arg,
  (function(Tv, Tv): num) $c,
)[]: bool;
<<__PHPStdLib>>
function uasort<Tk as arraykey, Tv>(

  inout KeyedContainer<Tk, Tv> $arg,
  (function(Tv, Tv): num) $c,
)[]: bool;
<<__PHPStdLib>>
function uksort<Tk as arraykey, Tv>(

  inout KeyedContainer<Tk, Tv> $arg,
  (function(Tk, Tk): num) $c,
)[]: bool;

}

namespace HH {

/**
 * Creates a `dict` from a `KeyedTraversable`, preserving keys and order.
 */
function dict<Tk as arraykey, Tv>(KeyedTraversable<Tk, Tv> $arr)[]: dict<Tk, Tv>;

/**
 * Creates a `vec` from a `Traversable`, preserving order. Keys are not
 * preserved.
 */
function vec<Tv>(Traversable<Tv> $arr)[]: vec<Tv>;

/**
 * Create a `keyset` from a `Traversable` of strings or ints, preserving order.
 * Keys are not preserved.
 */
function keyset<Tv as arraykey>(Traversable<Tv> $arr)[]: keyset<Tv>;

function darray<Tk as arraykey, Tv>(KeyedTraversable<Tk, Tv> $arr)[]: darray<Tk, Tv>;

function varray<Tv>(Traversable<Tv> $arr)[]: varray<Tv>;

function is_php_array(readonly mixed $input)[]: bool;

function is_vec_or_varray(readonly mixed $input)[]: bool;

function is_dict_or_darray(readonly mixed $input)[]: bool;

}

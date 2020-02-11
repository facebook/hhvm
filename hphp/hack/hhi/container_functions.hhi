<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
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

namespace {

<<__PHPStdLib, __Rx>>
function array_key_exists(mixed $key, <<__MaybeMutable>> ?KeyedContainer<arraykey, mixed> $search): bool;

<<__PHPStdLib, __Rx>>
function array_sum/*<T>*/(/*Container<T>*/ $input)/*: num*/;
<<__PHPStdLib, __Rx>>
function array_product/*<T>*/(/*Container<T>*/ $input)/*: num*/;

<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function sort<T as Container<mixed>>(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>> inout T $arg,
  int $sort_flags = SORT_REGULAR,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function rsort<T as Container<mixed>>(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>> inout T $arg,
  int $sort_flags = SORT_REGULAR,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function asort<Tk as arraykey, Tv>(
  <<__OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> inout KeyedContainer<Tk, Tv> $arg,
  int $sort_flags = SORT_REGULAR,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function arsort<Tk as arraykey, Tv>(
  <<__OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> inout KeyedContainer<Tk, Tv> $arg,
  int $sort_flags = SORT_REGULAR,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function ksort<T as KeyedContainer<arraykey, mixed>>(
  <<__OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> inout T $arg,
  int $sort_flags = SORT_REGULAR,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function krsort<Tk as arraykey, Tv>(
  <<__OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> inout KeyedContainer<Tk, Tv> $arg,
  int $sort_flags = SORT_REGULAR,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function usort<Tv, T as Container<Tv>>(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>> inout T $arg,
  <<__AtMostRxAsFunc>> (function(Tv, Tv): num) $c,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function uasort<Tk as arraykey, Tv>(
  <<__OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> inout KeyedContainer<Tk, Tv> $arg,
  <<__AtMostRxAsFunc>> (function(Tv, Tv): num) $c,
): bool;
<<__PHPStdLib, __Rx, __AtMostRxAsArgs>>
function uksort<Tk as arraykey, Tv>(
  <<__OnlyRxIfImpl(\HH\Rx\KeyedTraversable::class)>> inout KeyedContainer<Tk, Tv> $arg,
  <<__AtMostRxAsFunc>> (function(Tk, Tk): num) $c,
): bool;

}

namespace HH {

/**
 * Creates a `dict` from a `KeyedTraversable`, preserving keys and order.
 */
<<__Rx, __AtMostRxAsArgs>>
function dict<Tk as arraykey, Tv>(<<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>> KeyedTraversable<Tk, Tv> $arr): dict<Tk, Tv>;

/**
 * Creates a `vec` from a `Traversable`, preserving order. Keys are not
 * preserved.
 */
<<__Rx, __AtMostRxAsArgs>>
function vec<Tv>(<<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>> Traversable<Tv> $arr): vec<Tv>;

/**
 * Create a `keyset` from a `Traversable` of strings or ints, preserving order.
 * Keys are not preserved.
 */
<<__Rx, __AtMostRxAsArgs>>
function keyset<Tv as arraykey>(<<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>> Traversable<Tv> $arr): keyset<Tv>;

<<__Rx>>
function darray<Tk as arraykey, Tv>(KeyedTraversable<Tk, Tv> $arr): darray<Tk, Tv>;

<<__Rx>>
function varray<Tv>(Traversable<Tv> $arr): varray<Tv>;

<<__Rx>>
function is_php_array(<<__MaybeMutable>> mixed $input): bool;


}

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

  <<__PHPStdLib>>
  function array_key_exists(
    readonly mixed $key,
    readonly ?KeyedContainer<arraykey, mixed> $search,
  )[]: bool;

  <<__PHPStdLib>>
  function array_sum/*<T>*/(
    readonly /*Container<T>*/ HH\FIXME\MISSING_PARAM_TYPE $input,
  )[]/*: num*/: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function array_product/*<T>*/(
    readonly /*Container<T>*/ HH\FIXME\MISSING_PARAM_TYPE $input,
  )[]/*: num*/: \HH\FIXME\MISSING_RETURN_TYPE;

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
    (function(Tv, Tv)[_]: num) $c,
  )[ctx $c]: bool;
  <<__PHPStdLib>>
  function uasort<Tk as arraykey, Tv>(

    inout KeyedContainer<Tk, Tv> $arg,
    (function(Tv, Tv)[_]: num) $c,
  )[ctx $c]: bool;
  <<__PHPStdLib>>
  function uksort<Tk as arraykey, Tv>(

    inout KeyedContainer<Tk, Tv> $arg,
    (function(Tk, Tk)[_]: num) $c,
  )[ctx $c]: bool;

}

namespace HH {

  /**
   * Creates a `dict` from a `KeyedTraversable`, preserving keys and order.
   */
  <<__NoAutoDynamic, __SupportDynamicType>>
  function dict<Tk as arraykey, Tv as FIXME\SUPPORTDYN_MARKER<mixed>>(
    KeyedTraversable<Tk, Tv> $arr,
  )[]: dict<Tk, Tv>;

  /**
   * Creates a `vec` from a `Traversable`, preserving order. Keys are not
   * preserved.
   */
  <<__NoAutoDynamic, __SupportDynamicType>>
  function vec<Tv as FIXME\SUPPORTDYN_MARKER<mixed>>(Traversable<Tv> $arr)[]: vec<Tv>;

  /**
   * Create a `keyset` from a `Traversable` of strings or ints, preserving order.
   * Keys are not preserved.
   */
  <<__NoAutoDynamic, __SupportDynamicType>>
  function keyset<Tv as arraykey>(Traversable<Tv> $arr)[]: keyset<Tv>;

  <<__NoAutoDynamic, __SupportDynamicType>>
  function darray<Tk as arraykey, Tv as FIXME\SUPPORTDYN_MARKER<mixed>>(
    KeyedTraversable<Tk, Tv> $arr,
  )[]: darray<Tk, Tv>;

  <<__NoAutoDynamic, __SupportDynamicType>>
  function varray<Tv as FIXME\SUPPORTDYN_MARKER<mixed>>(Traversable<Tv> $arr)[]: varray<Tv>;

  function is_php_array(readonly mixed $input)[]: bool;

  function is_vec_or_varray(readonly mixed $input)[]: bool;

  function is_dict_or_darray(readonly mixed $input)[]: bool;

}

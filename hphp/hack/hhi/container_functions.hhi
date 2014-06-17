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
function array_key_exists<Tk, Tv>(mixed $key, ?KeyedContainer<Tk, Tv> $search): bool;

function sort<Tv>(Container<Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
function rsort<Tv>(Container<Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
function asort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
function arsort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR, bool $intl_sort = false): bool;
function ksort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR): bool;
function krsort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, int $sort_flags = SORT_REGULAR): bool;
// $c is a callable of type (function(Tv,Tv): bool)
function usort<Tv>(Container<Tv> &$arg, mixed $c): bool;
// $c is a callable of type (function(Tv,Tv): bool)
function uasort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, mixed $c): bool;
// $c is a callable of type (function(Tk,Tk): bool)
function uksort<Tk,Tv>(KeyedContainer<Tk, Tv> &$arg, mixed $c): bool;

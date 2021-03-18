<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

/**
 * C is for Containers. This file contains functions that ask
 * questions of (i.e. introspect) containers and traversables.
 */
namespace HH\Lib\C;

/**
 * Returns true if the given predicate returns true for any element of the
 * given Traversable. If no predicate is provided, it defaults to casting the
 * element to bool.
 *
 * If you're looking for `C\none`, use `!C\any`.
 *
 * Time complexity: O(n)
 * Space complexity: O(1)
 */
function any<T>(
  Traversable<T> $traversable,
  ?(function(T)[_]: bool) $predicate = null,
)[ctx $predicate]: bool {
  $predicate ??= \HH\Lib\_Private\boolval<>;
  foreach ($traversable as $value) {
    if ($predicate($value)) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if the given Traversable contains the value. Strict equality is
 * used.
 *
 * Time complexity: O(n) (O(1) for keysets)
 * Space complexity: O(1)
 */
function contains<T1, T2>(
  Traversable<T1> $traversable,
  T2 $value,
)[]: bool {
  if ($traversable is keyset<_>) {
    return $value is arraykey && contains_key($traversable, $value);
  }
  foreach ($traversable as $v) {
    if ($value === $v) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if the given KeyedContainer contains the key.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function contains_key<Tk1 as arraykey, Tk2 as arraykey, Tv>(
  KeyedContainer<Tk1, Tv> $container,
  Tk2 $key,
)[]: bool {
  /* HH_FIXME[2049] __PHPStdLib */
  /* HH_FIXME[4107] __PHPStdLib */
  return \array_key_exists($key, $container);
}

/**
 * Returns the number of elements in the given Container.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function count(
  Container<mixed> $container,
)[]: int {
  /* HH_FIXME[2049] __PHPStdLib */
  /* HH_FIXME[4107] __PHPStdLib */
  return \count($container);
}

/**
 * Returns true if the given predicate returns true for every element of the
 * given Traversable. If no predicate is provided, it defaults to casting the
 * element to bool.
 *
 * If you're looking for `C\all`, this is it.
 *
 * Time complexity: O(n)
 * Space complexity: O(1)
 */
function every<T>(
  Traversable<T> $traversable,
  ?(function(T)[_]: bool) $predicate = null,
)[ctx $predicate]: bool {
  $predicate ??= \HH\Lib\_Private\boolval<>;
  foreach ($traversable as $value) {
    if (!$predicate($value)) {
      return false;
    }
  }
  return true;
}

/**
 * Returns whether the given Container is empty.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function is_empty<T>(
  Container<T> $container,
)[]: bool {
  if ($container is \ConstCollection<_>) {
    return $container->isEmpty();
  }
  return !$container;
}

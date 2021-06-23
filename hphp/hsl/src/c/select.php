<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\C;

use namespace HH\Lib\{_Private, Str};

/**
 * Returns the first value of the given Traversable for which the predicate
 * returns true, or null if no such value is found.
 *
 * Time complexity: O(n)
 * Space complexity: O(1)
 *
 * @see `C\findx` when a value is required
 */
function find<T>(
  Traversable<T> $traversable,
  (function(T)[_]: bool) $value_predicate,
)[ctx $value_predicate]: ?T {
  foreach ($traversable as $value) {
    if ($value_predicate($value)) {
      return $value;
    }
  }
  return null;
}

/**
 * Returns the first value of the given Traversable for which the predicate
 * returns true, or throws if no such value is found.
 *
 * Time complexity: O(n)
 * Space complexity: O(1)
 *
 * @see `C\find()` if you would prefer null if not found.
 */
function findx<T>(
  Traversable<T> $traversable,
  (function(T)[_]: bool) $value_predicate,
)[ctx $value_predicate]: T {
  foreach ($traversable as $value) {
    if ($value_predicate($value)) {
      return $value;
    }
  }
  invariant_violation('%s: Couldn\'t find target value.', __FUNCTION__);
}

/**
 * Returns the key of the first value of the given KeyedTraversable for which
 * the predicate returns true, or null if no such value is found.
 *
 * Time complexity: O(n)
 * Space complexity: O(1)
 */
function find_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tv)[_]: bool) $value_predicate,
)[ctx $value_predicate]: ?Tk {
  foreach ($traversable as $key => $value) {
    if ($value_predicate($value)) {
      return $key;
    }
  }
  return null;
}

/**
 * Returns the first element of the given Traversable, or null if the
 * Traversable is empty.
 *
 * - For non-empty Traversables, see `C\firstx`.
 * - For possibly null Traversables, see `C\nfirst`.
 * - For single-element Traversables, see `C\onlyx`.
 * - For Awaitables that yield Traversables, see `C\first_async`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function first<T>(
  Traversable<T> $traversable,
)[]: ?T {
  if ($traversable is Container<_>) {
    return _Private\Native\first($traversable);
  }
  foreach ($traversable as $value) {
    return $value;
  }
  return null;
}

/**
 * Returns the first element of the given Traversable, or throws if the
 * Traversable is empty.
 *
 * - For possibly empty Traversables, see `C\first`.
 * - For possibly null Traversables, see `C\nfirst`.
 * - For single-element Traversables, see `C\onlyx`.
 * - For Awaitables that yield Traversables, see `C\firstx_async`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function firstx<T>(
  Traversable<T> $traversable,
)[]: T {
  if ($traversable is Container<_>) {
    $first_value = _Private\Native\first($traversable);
    if ($first_value is nonnull) {
      return $first_value;
    }
    invariant(
      !is_empty($traversable),
      '%s: Expected at least one element.',
      __FUNCTION__,
    );
    /* HH_FIXME[4110] invariant above implies this is T */
    return $first_value;
  }
  foreach ($traversable as $value) {
    return $value;
  }
  invariant_violation('%s: Expected at least one element.', __FUNCTION__);
}

/**
 * Returns the first key of the given KeyedTraversable, or null if the
 * KeyedTraversable is empty.
 *
 * For non-empty Traversables, see `C\first_keyx`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function first_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
)[]: ?Tk {
  if ($traversable is KeyedContainer<_, _>) {
    return _Private\Native\first_key($traversable);
  }
  foreach ($traversable as $key => $_) {
    return $key;
  }
  return null;
}

/**
 * Returns the first key of the given KeyedTraversable, or throws if the
 * KeyedTraversable is empty.
 *
 * For possibly empty Traversables, see `C\first_key`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function first_keyx<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
)[]: Tk {
  if ($traversable is KeyedContainer<_, _>) {
    $first_key = _Private\Native\first_key($traversable);
    invariant(
      $first_key is nonnull,
      '%s: Expected at least one element.',
      __FUNCTION__,
    );
    return $first_key;
  }
  foreach ($traversable as $key => $_) {
    return $key;
  }
  invariant_violation('%s: Expected at least one element.', __FUNCTION__);
}

/**
 * Returns the last element of the given Traversable, or null if the
 * Traversable is empty.
 *
 * - For non-empty Traversables, see `C\lastx`.
 * - For single-element Traversables, see `C\onlyx`.
 *
 * Time complexity: O(1) if `$traversable` is a `Container`, O(n) otherwise.
 * Space complexity: O(1)
 */
function last<T>(
  Traversable<T> $traversable,
)[]: ?T {
  if ($traversable is Container<_>) {
    return _Private\Native\last($traversable);
  }
  if ($traversable is Iterable<_>) {
    /* HH_FIXME[4390] need ctx constants */
    return $traversable->lastValue();
  }
  $value = null;
  foreach ($traversable as $value) {
  }
  return $value;
}

/**
 * Returns the last element of the given Traversable, or throws if the
 * Traversable is empty.
 *
 * - For possibly empty Traversables, see `C\last`.
 * - For single-element Traversables, see `C\onlyx`.
 *
 * Time complexity: O(1) if `$traversable` is a `Container`, O(n) otherwise.
 * Space complexity: O(1)
 */
function lastx<T>(
  Traversable<T> $traversable,
)[]: T {
  if ($traversable is Container<_>) {
    $last_value = _Private\Native\last($traversable);
    if ($last_value is nonnull) {
      return $last_value;
    }
    invariant(
      !is_empty($traversable),
      '%s: Expected at least one element.',
      __FUNCTION__,
    );
    /* HH_FIXME[4110] invariant above implies this is T */
    return $last_value;
  }
  $value = null;
  $did_iterate = false;
  foreach ($traversable as $value) {
    $did_iterate = true;
  }
  invariant($did_iterate, '%s: Expected at least one element.', __FUNCTION__);
  /* HH_FIXME[4110] invariant above implies this is T */
  return $value;
}

/**
 * Returns the last key of the given KeyedTraversable, or null if the
 * KeyedTraversable is empty.
 *
 * For non-empty Traversables, see `C\last_keyx`.
 *
 * Time complexity: O(1) if `$traversable` is a `Container`, O(n) otherwise.
 * Space complexity: O(1)
 */
function last_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
)[]: ?Tk {
  if ($traversable is KeyedContainer<_, _>) {
    return _Private\Native\last_key($traversable);
  }
  if ($traversable is KeyedIterable<_, _>) {
    /* HH_FIXME[4390] need ctx constants */
    return $traversable->lastKey();
  }
  $key = null;
  foreach ($traversable as $key => $_) {
  }
  return $key;
}

/**
 * Returns the last key of the given KeyedTraversable, or throws if the
 * KeyedTraversable is empty.
 *
 * For possibly empty Traversables, see `C\last_key`.
 *
 * Time complexity: O(1) if `$traversable` is a `Container`, O(n) otherwise.
 * Space complexity: O(1)
 */
function last_keyx<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
)[]: Tk {
  if ($traversable is KeyedContainer<_, _>) {
    $last_key = _Private\Native\last_key($traversable);
    invariant(
      $last_key is nonnull,
      '%s: Expected at least one element.',
      __FUNCTION__,
    );
    return $last_key;
  }
  $key = null;
  $did_iterate = false;
  foreach ($traversable as $key => $_) {
    $did_iterate = true;
  }
  invariant($did_iterate, '%s: Expected at least one element.', __FUNCTION__);
  /* HH_FIXME[4110] invariant above implies this is Tk */
  return $key;
}

/**
 * Returns the first element of the given Traversable, or null if the
 * Traversable is null or empty.
 *
 * - For non-null Traversables, see `C\first`.
 * - For non-empty Traversables, see `C\firstx`.
 * - For single-element Traversables, see `C\onlyx`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function nfirst<T>(
  ?Traversable<T> $traversable,
)[]: ?T {
  return $traversable is nonnull ? first($traversable) : null;
}

/**
 * Returns the first and only element of the given Traversable, or throws if the
 * Traversable is empty or contains more than one element.
 *
 * An optional format string (and format arguments) may be passed to specify
 * a custom message for the exception in the error case.
 *
 * For Traversables with more than one element, see `C\firstx`.
 *
 * Time complexity: O(1)
 * Space complexity: O(1)
 */
function onlyx<T>(
  Traversable<T> $traversable,
  ?Str\SprintfFormatString $format_string = null,
  mixed ...$format_args
)[]: T {
  $first = true;
  $result = null;
  foreach ($traversable as $value) {
    invariant(
      $first,
      '%s',
      $format_string === null
        ? Str\format(
          'Expected exactly one element%s.',
          $traversable is Container<_>
            ? ' but got '.count($traversable)
            : '',
        )
        : \vsprintf($format_string, $format_args),
    );
    $result = $value;
    $first = false;
  }
  invariant(
    $first === false,
    '%s',
    $format_string === null
      ? 'Expected non-empty Traversable.'
      : \vsprintf($format_string, $format_args),
  );
  /* HH_FIXME[4110] $first is false implies $result is set to T */
  return $result;
}

/**
 * Removes the last element from a Container and returns it.
 * If the Container is empty, null will be returned.
 *
 * When an immutable Hack Collection is passed, the result will
 * be defined by your version of hhvm and not give the expected results.
 *
 * For non-empty Containers, see `pop_backx`.
 * To get the first element, see `pop_front`.
 *
 * Time complexity: O(1 or N) If the operation can happen in-place, O(1)
 *   if it must copy the Container, O(N).
 * Space complexity: O(1 or N) If the operation can happen in-place, O(1)
 *   if it must copy the Container, O(N).
 */
function pop_back<T as Container<Tv>, Tv>(
  inout T $container,
)[]: ?Tv {
  if (is_empty($container)) {
    return null;
  }
  return \array_pop(inout $container);
}

/**
 * Removes the last element from a Container and returns it.
 * If the Container is empty, an `InvariantException` is thrown.
 *
 * When an immutable Hack Collection is passed, the result will
 * be defined by your version of hhvm and not give the expected results.
 *
 * For maybe empty Containers, see `pop_back`.
 * To get the first element, see `pop_frontx`.
 *
 * Time complexity: O(1 or N) If the operation can happen in-place, O(1)
 *   if it must copy the Container, O(N).
 * Space complexity: O(1 or N) If the operation can happen in-place, O(1)
 *   if it must copy the Container, O(N).
 */
function pop_backx<T as Container<Tv>, Tv>(
  inout T $container,
)[]: Tv {
  invariant(
    !is_empty($container),
    '%s: Expected at least one element',
    __FUNCTION__,
  );
  return \array_pop(inout $container);
}

/**
 * Like `pop_back`, but removes the first item.
 *
 * Removes the first element from a Container and returns it.
 * If the Container is empty, null is returned.
 *
 * When an immutable Hack Collection is passed, the result will
 * be defined by your version of hhvm and not give the expected results.
 *
 * To enforce that the container is not empty, see `pop_frontx`.
 * To get the last element, see `pop_back`.
 *
 * Note that removing an item from the input array may not be "cheap." Keyed
 * containers such as `dict` can easily have the first item removed, but indexed
 * containers such as `vec` need to be wholly rewritten so the new [0] is the
 * old [1].
 *
 * Time complexity: O(1 or N): If the operation can happen in-place, O(1);
 *   if it must copy the Container, O(N).
 * Space complexity: O(1 or N): If the operation can happen in-place, O(1);
 *   if it must copy the Container, O(N).
 */
function pop_front<T as Container<Tv>, Tv>(inout T $container): ?Tv {
  if (is_empty($container)) {
    return null;
  }
  return \array_shift(inout $container);
}

/**
 * Like `pop_front` but enforces non-empty container as input.
 */
function pop_frontx<T as Container<Tv>, Tv>(inout T $container): Tv {
  invariant(
    !is_empty($container),
    '%s: Expected at least one element',
    __FUNCTION__,
  );
  return \array_shift(inout $container);
}

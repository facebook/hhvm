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
 * This file provides type information for some of HHVM's builtin classes.
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

/**
 * ImmMap is an immutable Map. HHVM provides a native implementation for this
 * class. The PHP class definition below is not actually used at run time; it
 * is simply provided for the typechecker and for developer reference.
 *
 * A ImmMap cannot be mutated. No elements can be added or removed from it,
 * nor can elements be overwritten using assignment (i.e. "$c[$k] = $v" is
 * not allowed).
 *
 * Construct it with a Traversable
 *
 *   $a = array('a' => 1, 'b' => 2);
 *   $fm = new ImmMap($a);
 *
 * or use the literal syntax
 *
 *   $fm = ImmMap {'a' => 1, 'b' => 2};
 */

final class ImmMap<Tk, +Tv> implements ConstMap<Tk, Tv>, Indexish<Tk, Tv> {
  /**
   * Creates an ImmMap from the given KeyedTraversable, or an empty ImmMap
   * if "null" is passed.
   */
  public function __construct(?KeyedTraversable<Tk, Tv> $it);

  /**
   * Returns an array containing the key/value pairs from this ImmMap
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this ImmMap.
   */
  public function toValuesArray(): array;

  /**
   * Returns an array whose values are this ImmMap's keys.
   */
  public function toKeysArray(): array;

  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toMap(): Map<Tk, Tv>;
  public function toImmMap(): ImmMap<Tk, Tv>;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;
  public function immutable(): ImmMap<Tk, Tv>;
  public function lazy(): KeyedIterable<Tk, Tv>;
  public function values(): ImmVector<Tv>;
  public function keys(): ImmVector<Tk>;
  public function map<Tu>((function(Tv): Tu) $callback): ImmMap<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $callback):
    ImmMap<Tk, Tu>;
  public function filter((function(Tv): bool) $callback): ImmMap<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $callback):
    ImmMap<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    ImmMap<Tk, Pair<Tv, Tu>>;
  public function take(int $n): ImmMap<Tk, Tv>;
  public function takeWhile((function(Tv): bool) $fn): ImmMap<Tk, Tv>;
  public function skip(int $n): ImmMap<Tk, Tv>;
  public function skipWhile((function(Tv): bool) $fn): ImmMap<Tk, Tv>;
  public function slice(int $start, int $len): ImmMap<Tk, Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): ImmVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;

  /**
   * Returns true if the ImmMap is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of key/value pairs in the ImmMap.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key. If the key is not present,
   * an exception is thrown. "$v = $m->at($k)" is semantically equivalent
   * to "$v = $m[$k]".
   */
  public function at(Tk $k): Tv;

  /**
   * Returns the value at the specified key. If the key is not present,
   * null is returned.
   */
  public function get(Tk $k): ?Tv;

  /**
   * Returns true if the specified key is present in the ImmMap, false
   * otherwise.
   */
  public function contains<Tu super Tk>(Tu $k): bool;
  public function containsKey<Tu super Tk>(Tu $k): bool;

  /**
   * Returns a new Map with the keys in this Map not in $traversable
   */
  public function differenceByKey<Tu super Tk, Tw>(
    KeyedTraversable<Tu, Tw> $traversable
  ) /*: Map<Tk, Tv> */;

  /**
   * Returns an iterator that points to the beginning of this ImmMap.
   */
  public function getIterator(): MapIterator<Tk, Tv>;

  public static function fromItems(?Traversable<Pair<Tk, Tv>> $items):
    ImmMap<Tk, Tv>;

  public function __toString(): string;

  public function items(): Iterable<Pair<Tk, Tv>>;
}

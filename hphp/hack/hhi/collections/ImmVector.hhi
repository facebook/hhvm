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
 * ImmVector is an immutable Vector. HHVM provides a native implementation for
 * this class. The PHP class definition below is not actually used at run time;
 * it is simply provided for the typechecker and for developer reference.
 *
 * A ImmVector cannot be mutated. No elements can be added or removed from it,
 * nor can elements be overwritten using assignment (i.e. "$c[$k] = $v" is not
 * allowed).
 *
 *   $v = Vector {1, 2, 3};
 *   $fv = $v->toImmVector();
 *
 * construct it with a Traversable
 *
 *   $a = array(1, 2, 3);
 *   $fv = new ImmVector($a);
 *
 * or use the literal syntax
 *
 *   $fv = ImmVector {1, 2, 3};
 */

final class ImmVector<+Tv> implements ConstVector<Tv> {
  /**
   * Creates an ImmVector from the given Traversable, or an empty ImmVector
   * if "null" is passed.
   */
  public function __construct(?Traversable<Tv> $it);

  /**
   * Returns true if the ImmVector is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of elements in this ImmVector.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key. If the key is not present,
   * an exception is thrown. "$v = $fvec->at($k)" is semantically equivalent
   * to "$v = $fvec[$k]".
   */
  public function at(int $k): Tv;

  /**
   * Returns the value at the specified key. If the key is not present,
   * null is returned.
   */
  public function get(int $k): ?Tv;

  /**
   * Returns true if the specified key is present in the ImmVector, returns
   * false otherwise.
   */
  public function containsKey<Tu super int>(Tu $k): bool;

  /**
   * Returns an array containing the values from this ImmVector.
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this ImmVector.
   */
  public function toValuesArray(): array;

  /**
   * Returns an array whose values are the keys from this ImmVector.
   */
  public function toKeysArray(): array;

  /**
   * Returns an iterator that points to the beginning of this ImmVector.
   */
  public function getIterator(): VectorIterator<Tv>;

  /**
   * Returns the index of the first element that matches the search value.
   * If no element matches the search value, this function returns -1.
   */
  public function linearSearch<Tu super Tv>(Tu $search_value): int;

  public static function fromItems(?Traversable<Tv> $items): ImmVector<Tv>;

  /**
   * Returns an ImmVector built from the keys of the specified container.
   */
  public static function fromKeysOf<Tk>(
    ?KeyedContainer<Tk, mixed> $container,
  ): ImmVector<Tk>;

  public function __toString(): string;

  public function items(): Iterable<Tv>;

  /**
   * Returns a copy of this ImmVector.
   */
  public function toImmVector(): ImmVector<Tv>;

  /**
   * Returns a Vector containing the elements of this ImmVector.
   */
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv>;

  /**
   * Returns a Map containing the elements of this ImmVector.
   * The keys are 0... count() - 1.
   */
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toMap(): Map<int, Tv>;
  public function toImmMap(): ImmMap<int, Tv>;

  /**
   * Returns a Set containing the elements of this ImmVector.
   */
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;

  public function immutable(): ImmVector<Tv>;
  public function lazy(): KeyedIterable<int, Tv>;
  public function values(): ImmVector<Tv>;
  public function keys(): ImmVector<int>;
  public function map<Tu>((function(Tv): Tu) $callback): ImmVector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $callback):
    ImmVector<Tu>;
  public function filter((function(Tv): bool) $callback): ImmVector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $callback):
    ImmVector<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    ImmVector<Pair<Tv, Tu>>;
  public function take(int $n): ImmVector<Tv>;
  public function takeWhile((function(Tv): bool) $fn): ImmVector<Tv>;
  public function skip(int $n): ImmVector<Tv>;
  public function skipWhile((function(Tv): bool) $fn): ImmVector<Tv>;
  public function slice(int $start, int $len): ImmVector<Tv>;
  public function concat<Tu super Tv>(
    Traversable<Tu> $traversable
  ): ImmVector<Tv>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?int;
  public function lastValue(): ?Tv;
  public function lastKey(): ?int;
}

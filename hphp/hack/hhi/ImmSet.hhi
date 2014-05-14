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

final class ImmSet<Tv> implements ConstSet<Tv> {
  /**
   * Create an empty ImmSet (if no parameters are passed) or create
   * an ImmSet from an Traversable (if one parameter is passed).
   */
  public function __construct(?Traversable<Tv> $it = null);

  /**
   * Returns true if the ImmSet is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of elements in this ImmSet.
   */
  public function count(): int;

  /**
   * Returns true if the specified value is present in the ImmSet, returns
   * false otherwise.
   */
  public function contains(Tv $k): bool;

  /**
   * Returns an array containing the values from this ImmSet.
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this ImmSet.
   */
  public function toValuesArray(): array;

  /**
   * Returns an iterator that points to the beginning of this ImmSet.
   */
  public function getIterator(): SetIterator<Tv>;

  /**
   * Returns an ImmSet containing the values from the specified array.
   */
  public static function fromArray(array $arr): ImmSet<Tv>;

  public static function fromItems(?Traversable<Tv> $items): ImmSet<Tv>;

  public static function fromArrays(...): ImmSet<Tv>;

  /**
   * Returns a ImmSet built from the keys of the specified container.
   */
  public static function fromKeysOf<Tk, Tv2>(
    ?KeyedContainer<Tk,Tv2> $container
  ): ImmSet<Tk>;

  public function __toString(): string;

  /**
   * Returns a copy of this ImmSet.
   */
  public function toImmSet(): ImmSet<Tv>;

  /**
   * Returns a Vector containing the elements of this ImmSet.
   */
  public function toVector(): Vector<Tv>;

  /**
   * Returns an ImmVector built from the values of this ImmSet.
   */
  public function toImmVector(): ImmVector<Tv>;

  /**
   * Returns a Set containing the elements of this ImmSet.
   */
  public function toSet(): Set<Tv>;

  public function items(): Iterable<Tv>;
  public function values(): ImmVector<Tv>;
  public function lazy(): Iterable<Tv>;
  public function map<Tu>((function(Tv): Tu) $callback): ImmSet<Tu>;
  public function filter((function(Tv): bool) $callback): ImmSet<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): ImmSet<Pair<Tv, Tu>>;
  public function take(int $n): ImmSet<Tv>;
  public function takeWhile((function(Tv): bool) $fn): ImmSet<Tv>;
  public function skip(int $n): ImmSet<Tv>;
  public function skipWhile((function(Tv): bool) $fn): ImmSet<Tv>;
  public function slice(int $start, int $len): ImmSet<Tv>;
  public function concat(Traversable<Tv> $traversable): ImmVector<Tv>;
  public function firstValue(): ?Tv;
  public function lastValue(): ?Tv;
}

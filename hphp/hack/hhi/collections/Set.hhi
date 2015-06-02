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
 * Set is an ordered set-style collection. HHVM provides a native
 * implementation for this class. The PHP class definition below is not
 * actually used at run time; it is simply provided for the typechecker and
 * for developer reference.
 *
 * Like all objects in PHP, Sets have reference-like semantics. When a caller
 * passes a Set to a callee, the callee can modify the Set and the caller will
 * see the changes. Sets do not have "copy-on-write" semantics.
 *
 * Sets preserve insertion order of the elements. When iterating over a Set,
 * the elements appear in the order they were inserted. Also, Sets do not
 * automagically convert integer-like strings (ex. "123") into integers.
 *
 * Sets only support integer values and string values. If a value of a
 * different type is used, an exception will be thrown.
 *
 * In general, Sets do not support "$c[$k]" style syntax. Adding an element
 * using "$c[] = .." syntax is supported.
 *
 * Set do not support iteration while elements are being added or removed.
 * When an element is added or removed, all iterators that point to the Set
 * shall be considered invalid.
 *
 * Sets do not support taking elements by reference. If binding assignment (=&)
 * is used when adding a new element to a Set (ex. "$c[] =& ..."), or if a Set
 * is used with foreach by reference, an exception will be thrown.
 */

final class Set<Tv> implements MutableSet<Tv> {
  /**
   * Creates a Set from the given Traversable, or an empty Set
   * if "null" is passed.
   */
  public function __construct(?Traversable<Tv> $it);

  /**
   * Returns an array containing the values from this Set.
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this Set.
   */
  public function toKeysArray(): array;

  /**
   * Returns an array containing the values from this Set.
   */
  public function toValuesArray(): array<Tv>;

  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
  public function toMap(): Map<mixed, Tv>;
  public function toImmMap(): ImmMap<mixed, Tv>;
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;
  public function immutable(): ImmSet<Tv>;
  public function lazy(): KeyedIterable<mixed, Tv>;
  public function values(): Vector<Tv>;
  public function keys(): Vector<mixed>;
  public function map<Tu>((function(Tv): Tu) $callback): Set<Tu>;
  public function mapWithKey<Tu>((function(mixed, Tv): Tu) $callback): Set<Tu>;
  public function filter((function(Tv): bool) $callback): Set<Tv>;
  public function filterWithKey((function(mixed, Tv): bool) $callback): Set<Tv>;

  /**
   * Ensures that this Set contains only members for which
   * the $callback returns a truthy result.
   */
  public function retain((function(Tv): bool) $callback): Set<Tv>;
  public function retainWithKey((function(mixed, Tv): bool) $callback): Set<Tv>;

  public function zip<Tu>(Traversable<Tu> $traversable): Set<Pair<Tv, Tu>>;
  public function take(int $n): Set<Tv>;
  public function takeWhile((function(Tv): bool) $fn): Set<Tv>;
  public function skip(int $n): Set<Tv>;
  public function skipWhile((function(Tv): bool) $fn): Set<Tv>;
  public function slice(int $start, int $len): Set<Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Vector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): mixed;
  public function lastValue(): ?Tv;
  public function lastKey(): mixed;

  /**
   * Returns true if the Set is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of elements in the Set.
   */
  public function count(): int;

  /**
   * Removes all elements from the Set and returns itself.
   */
  public function clear(): Set<Tv>;

  /**
   * Returns true if the specified value is present in the Set, false otherwise.
   */
  public function contains<Tu super Tv>(Tu $v): bool;

  /**
   * Adds an element to this Set and returns itself. "$c->add($v)" is
   * equivalent to "$c[] = $v" (except that add() returns the Set).
   */
  public function add(Tv $v): Set<Tv>;

  public function addAll(?Traversable<Tv> $it): Set<Tv>;

  /**
   * Adds the keys of the specified container to this Set and returns itself.
   */
  public function addAllKeysOf<Tv2>(
    ?KeyedContainer<Tv,Tv2> $container,
  ): Set<Tv>;

  /**
   * Reserves enough memory to accommodate 'sz' elements. If 'sz' is less
   * than or equal to the current capacity of this Set, does nothing.
   */
  public function reserve(int $sz): void;

  /**
   * Removes the specified value from this Set and returns itself.
   */
  public function remove(Tv $v): Set<Tv>;

  public function removeAll(Traversable<Tv> $other): Set<Tv>;

  /**
   * Returns an iterator that points to the beginning of this Set.
   */
  public function getIterator(): SetIterator<Tv>;

  /**
   * Returns a Set containing the values from the specified array.
   */
  <<__Deprecated('Use `new Set()` instead.')>>
  public static function fromArray<T>(array<T, Tv> $arr): Set<Tv>;

  public static function fromArrays(...): Set<Tv>;

  public static function fromItems<Tv2>(?Traversable<Tv2> $items): Set<Tv2>;

  /**
   * Returns a Set built from the keys of the specified container.
   */
  public static function fromKeysOf<Tk, Tv2>(
    ?KeyedContainer<Tk,Tv2> $container,
  ): Set<Tk>;

  public function __toString(): string;

  public function items(): Iterable<Tv>;
}

class SetIterator<+Tv> implements KeyedIterator<mixed, Tv> {
  public function __construct();
  public function current(): Tv;
  public function key(): mixed;
  public function valid(): bool;
  public function next(): void;
  public function rewind(): void;
}

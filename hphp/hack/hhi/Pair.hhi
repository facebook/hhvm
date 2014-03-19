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
 * Pair is a fixed-size collection with exactly two elements (possibly of
 * different types). HHVM provides a native implementation for this class.
 * The PHP class definition below is not actually used at run time; it is
 * simply provided for the typechecker and for developer reference.
 *
 * Like all objects in PHP, Pairs have reference-like semantics. The elements
 * or a Pair cannot be mutated (i.e. you can assign to the elements of a Pair)
 * though Pairs may contain mutable objects.
 *
 * Pairs only support integer keys. If a non-integer key is used, an
 * exception will be thrown.
 *
 * Pairs suoport "$m[$k]" style syntax for getting and setting values by
 * key. Pairs also support "isset($m[$k])" and "empty($m[$k])" syntax, and
 * they provide similar semantics as arrays. Elements can be added to a Pair
 * using "$m[] = .." syntax.
 *
 * Pairs do not support taking elements by reference. If binding assignment
 * (=&) is used with an element of a Pair, or if an element of a Pair is
 * passed by reference, of if a Pair is used with foreach by reference, an
 * exception will be thrown.
 */

final class Pair<Tv1, Tv2> implements ConstVector<mixed> {
  public function __construct();

  /**
   * Returns an array containing the values from this Pair.
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this Pair.
   */
  public function toValuesArray(): array;

  /**
   * Returns an array whose values are the keys from this Pair.
   */
  public function toKeysArray(): array;

  public function toVector(): Vector<mixed>;
  public function toImmVector(): ImmVector<mixed>;
  public function toMap(): Map<int, mixed>;
  public function toImmMap(): ImmMap<int, mixed>;
  public function toSet(): Set<mixed>;
  public function toImmSet(): ImmSet<mixed>;
  public function lazy(): KeyedIterable<int, mixed>;
  public function values(): Vector<mixed>;
  public function keys(): Vector<int>;
  public function map<Tu>((function(mixed): Tu) $callback): Vector<Tu>;
  public function mapWithKey<Tu>((function(int, mixed): Tu) $callback):
    Vector<Tu>;
  public function filter((function(mixed): bool) $callback): Vector<mixed>;
  public function filterWithKey((function(int, mixed): bool) $callback):
    Vector<mixed>;
  public function zip<Tu>(Traversable<Tu> $iterable): Vector<Pair<mixed, Tu>>;

  /**
   * Returns true if the Pair is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of elements in this Pair.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key. If the key is not present,
   * an exception is thrown. "$v = $p->at($k)" is semantically equivalent
   * to "$v = $p[$k]".
   */
  public function at(int $k): mixed;

  /**
   * Returns the value at the specified key. If the key is not present,
   * null is returned.
   */
  public function get(int $k): mixed;

  /**
   * Returns true if the specified key is present in the Pair, returns
   * false otherwise.
   */
  public function containsKey(int $k): bool;

  /**
   * Returns an iterator that points to beginning of this Pair.
   */
  public function getIterator(): KeyedIterator<int, mixed>;

  public function __toString(): string;
}

class PairIterator implements KeyedIterator<int, mixed> {
  public function __construct();
  public function rewind(): void;
  public function current(): mixed;
  public function key(): int;
  public function next(): void;
  public function valid(): bool;
}

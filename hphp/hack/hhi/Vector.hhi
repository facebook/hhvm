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
 * Vector is a stack-like collection. HHVM provides a native implementation
 * for this class. The PHP class definition below is not actually used at run
 * time; it is simply provided for the typechecker and for developer reference.
 *
 * Like all objects in PHP, Vectors have reference-like semantics. When a
 * caller passes a Vector to a callee, the callee can modify the Vector and the
 * caller will see the changes. Vectors do not have "copy-on-write" semantics.
 *
 * Vectors only support integer keys. If a non-integer key is used, an
 * exception will be thrown.
 *
 * Vectors suoport "$m[$k]" style syntax for getting and setting values by
 * key. Vectors also support "isset($m[$k])" and "empty($m[$k])" syntax, and
 * they provide similar semantics as arrays. Elements can be added to a Vector
 * using "$m[] = .." syntax.
 *
 * Vectors do not support iterating while new elements are being added or
 * elements are being removed. When a new element is added or removed, all
 * iterators that point to the Vector shall be considered invalid.
 *
 * Vectors do not support taking elements by reference. If binding assignment
 * (=&) is used with an element of a Vector, or if an element of a Vector is
 * passed by reference, of if a Vector is used with foreach by reference, an
 * exception will be thrown.
 */

final class Vector<Tv> implements MutableVector<Tv> {
  /**
   * Create an empty Vector (if no parameters are passed) or create
   * a Vector from an Traversable (if one parameter is passed).
   */
  public function __construct(?Traversable<Tv> $it);

  /**
   * Returns an array containing the values from this Vector.
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this Vector.
   */
  public function toValuesArray(): array;

  /**
   * Returns an array whose values are the keys from this Vector.
   */
  public function toKeysArray(): array;

  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
  public function toMap(): Map<int, Tv>;
  public function toImmMap(): ImmMap<int, Tv>;
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;
  public function lazy(): KeyedIterable<int, Tv>;
  public function values(): Vector<Tv>;
  public function keys(): Vector<int>;
  public function map<Tu>((function(Tv): Tu) $callback): Vector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $callback): Vector<Tu>;
  public function filter((function(Tv): bool) $callback): Vector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $callback):
    Vector<Tv>;
  public function zip<Tu>(Traversable<Tu> $iterable): Vector<Pair<Tv, Tu>>;

  /**
   * Returns true if the Vector is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of elements in this Vector.
   */
  public function count(): int;

  /**
   * Returns the value at the specified key. If the key is not present,
   * an exception is thrown. "$v = $vec->at($k)" is semantically equivalent
   * to "$v = $vec[$k]".
   */
  public function at(int $k): Tv;

  /**
   * Returns the value at the specified key. If the key is not present,
   * null is returned.
   */
  public function get(int $k): ?Tv;

  /**
   * Stores a value into the Vector with the specified key, overwriting the
   * previous value associated with the key. If the key is not present, an
   * an exception is thrown. "$vec->set($k,$v)" is semantically equivalent
   * to "$vec[$k] = $v" (except that set() returns the Vector).
   */
  public function set(int $k, Tv $v): Vector<Tv>;

  public function setAll(?KeyedTraversable<int, Tv> $it): Vector<Tv>;

  /**
   * Remove all the elements from this Vector.
   */
  public function clear(): Vector<Tv>;

  /**
   * Returns true if the specified key is present in the Vector, returns
   * false otherwise.
   */
  public function containsKey(int $k): bool;

  /**
   * Append a copy of a value to the end of the Vector, assigning the next
   * available integer key. "$vec->add($v)" is semantically equivalent to
   * "$vec[] = $v" (except that add() returns the Vector).
   */
  public function add(Tv $value): Vector<Tv>;

  public function addAll(?Traversable<Tv> $it): Vector<Tv>;

  /**
   * Removes the specified key from this Vector. This will cause elements
   * with higher keys to be renumbered.
   */
  public function removeKey(int $k): Vector<Tv>;

  /**
   * Remove the last element of this Vector and return it. This function
   * throws an exception if this Vector is empty.
   */
  public function pop(): Tv;

  /**
   * Resize this Vector to contain 'sz' elements. If 'sz' is smaller than
   * the current size of this Vector, elements are removed from the end of
   * this Vector. If 'sz' is greater than the current size of this Vector,
   * this Vector is extended by appending as many copies of 'value' as
   * needed to reach a size of 'sz' elements.
   */
  public function resize(int $sz, Tv $value): void;

  /**
   * Reserve memory for 'sz' elements. If 'sz' is smaller than the current
   * size of this Vector or 'sz' is zero, do nothing.
   */
  public function reserve(int $sz): void;

  /**
   * Returns an iterator that points to beginning of this Vector.
   */
  public function getIterator(): KeyedIterator<int, Tv>;

  /**
   * Reverse the elements of this Vector in place.
   */
  public function reverse(): void;

  /**
   * Splice this Vector in place. This function provides the functionality
   * of array_splice() for Vectors. Note that this function modifies this
   * Vector in place.
   */
  public function splice(int $offset, ?int $len = null): void;

  /**
   * Returns the index of the first element that matches the search value.
   * If no element matches the search value, this function returns -1.
   */
  public function linearSearch(Tv $search_value): int;

  /**
   * Shuffles the values of the Vector randomly in place.
   */
  public function shuffle(): void;

  /**
   * Returns a Vector containing the values from the specified array.
   */
  public static function fromArray<T>(array<T, Tv> $arr): Vector<Tv>;

  public static function fromItems(?Traversable<Tv> $items): Vector<Tv>;

  public function __toString(): string;

  public function items(): Iterable<Tv>;
}

class VectorIterator<Tv> implements KeyedIterator<int, Tv> {
  public function __construct();
  public function rewind(): void;
  public function current(): Tv;
  public function key(): int;
  public function next(): void;
  public function valid(): bool;
}

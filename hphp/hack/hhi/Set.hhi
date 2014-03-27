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
   * Create an empty Set (if no parameters are passed) or create
   * a Set from an Traversable (if one parameter is passed).
   */
  public function __construct(?Traversable<Tv> $it);

  /**
   * Returns an array containing the values from this Set.
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this Set.
   */
  public function toValuesArray(): array;

  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;
  public function lazy(): Iterable<Tv>;
  public function values(): Vector<Tv>;
  public function map<Tu>((function(Tv): Tu) $callback): Set<Tu>;
  public function filter((function(Tv): bool) $callback): Set<Tv>;
  public function zip<Tu>(Traversable<Tu> $iterable): Set<Pair<Tv, Tu>>;

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
  public function contains(Tv $v): bool;

  /**
   * Adds an element to this Set and returns itself. "$c->add($v)" is
   * equivalent to "$c[] = $v" (except that add() returns the Set).
   */
  public function add(Tv $v): Set<Tv>;

  public function addAll(?Traversable<Tv> $it): Set<Tv>;

  /**
   * Removes the specified value from this Set and returns itself.
   */
  public function remove(Tv $v): Set<Tv>;

  public function removeAll(Traversable<Tv> $other): Set<Tv>;

  /**
   * Returns an iterator that points to the beginning of this Set.
   */
  public function getIterator(): Iterator<Tv>;

  /**
   * Returns a Set containing the values from the specified array.
   */
  public static function fromArray(array $arr): Set<Tv>;

  public static function fromArrays(...): Set<Tv>;

  public static function fromItems(?Traversable<Tv> $items): Set<Tv>;

  public function __toString(): string;

  public function items(): Iterable<Tv>;
}

class SetIterator<Tv> implements Iterator<Tv> {
  public function __construct();
  public function current(): Tv;
  public function key(): mixed;
  public function valid(): bool;
  public function next(): void;
  public function rewind(): void;
}

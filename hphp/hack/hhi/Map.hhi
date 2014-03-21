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
 * Map is an ordered dictionary-style collection. HHVM provides a native
 * implementation for this class. The PHP class definition below is not
 * actually used at run time; it is simply provided for the typechecker and for
 * developer reference.
 *
 * Like all objects in PHP, Maps have reference-like semantics. When a caller
 * passes a Map to a callee, the callee can modify the Map and the caller will
 * see the changes. Maps do not have "copy-on-write" semantics.
 *
 * Maps preserve insertion order of key/value pairs. When iterating over a Map,
 * the key/value pairs appear in the order they were inserted. Also, Maps do
 * not automagically convert integer-like string keys (ex. "123") into integer
 * keys.
 *
 * Maps only support integer keys and string keys. If a key of a different
 * type is used, an exception will be thrown.
 *
 * Maps support "$m[$k]" style syntax for getting and setting values by key.
 * Maps also support "isset($m[$k])" and "empty($m[$k])" syntax, and they
 * provide similar semantics as arrays. Adding an element using "$m[] = .."
 * syntax is not supported.
 *
 * Maps do not support iterating while new keys are being added or elements
 * are being removed. When a new key is added or an element is removed, all
 * iterators that point to the Map shall be considered invalid.
 *
 * Maps do not support taking elements by reference. If binding assignment (=&)
 * is used with an element of a Map, or if an element of a Map is passed by
 * reference, of if a Map is used with foreach by reference, an exception will
 * be thrown.
 */

final class Map<Tk, Tv> implements MutableMap<Tk, Tv> {
  /**
   * Create an empty Map (if no parameters are passed) or create
   * a Map from an KeyedTraversable (if one parameter is passed).
   */
  public function __construct(?KeyedTraversable<Tk, Tv> $it);

  /**
   * Returns an array containing the key/value pairs from this Map
   */
  public function toArray(): array;

  /**
   * Returns an array containing the values from this Map.
   */
  public function toValuesArray(): array;

  /**
   * Returns an array whose values are this map's keys.
   */
  public function toKeysArray(): array;

  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
  public function toMap(): Map<Tk, Tv>;
  public function toImmMap(): ImmMap<Tk, Tv>;
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;
  public function lazy(): KeyedIterable<Tk, Tv>;
  public function values(): Vector<Tv>;
  public function keys(): Vector<Tk>;
  public function map<Tu>((function(Tv): Tu) $callback): Map<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $callback): Map<Tk, Tu>;
  public function filter((function(Tv): bool) $callback): Map<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $callback):
    Map<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $iterable): Map<Tk, Pair<Tv, Tu>>;

  /**
   * Returns true if the Map is empty, false otherwise.
   */
  public function isEmpty(): bool;

  /**
   * Returns the number of key/value pairs in the Map.
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
   * Stores a value into the Map with the specified key, overwriting any
   * previous value that was associated with the key. "$m->set($k,$v)" is
   * semantically equivalent to "$m[$k] = $v" (except that set() returns
   * the Map).
   */
  public function set(Tk $k, Tv $v): Map<Tk, Tv>;

  public function setAll(?KeyedTraversable<Tk, Tv> $it): Map<Tk, Tv>;

  /**
   * Removes all key/value pairs from the Map.
   */
  public function clear(): Map<Tk, Tv>;

  /**
   * Returns true if the specified key is present in the Map, false otherwise.
   */
  public function contains(Tk $k): bool;
  public function containsKey(Tk $k): bool;

  /**
   * Add a key/value Pair to this Map. "$mp->add($p)" is semantically
   * equivalent to "$mp[] = $p" (except that add() returns the Map).
   */
  public function add(Pair<Tk, Tv> $p): Map<Tk, Tv>;

  public function addAll(?Traversable<Pair<Tk, Tv>> $it): Map<Tk, Tv>;

  /**
   * Removes the specified key from this Map.
   */
  public function remove(Tk $k): Map<Tk, Tv>;
  public function removeKey(Tk $k): Map<Tk, Tv>;

  /**
   * Returns a new Map with the keys in this Map not in $iterable
   */
  public function differenceByKey(KeyedTraversable<Tk, Tv> $iterable): Map<Tk, Tv>;

  /**
   * Returns an iterator that points to beginning of this Map.
   */
  public function getIterator(): KeyedIterator<Tk, Tv>;

  /**
   * Returns a Map containing the key/value pairs from the specified array.
   */
  public static function fromArray(array $arr): Map<Tk, Tv>;

  public static function fromItems(?Traversable<Pair<Tk, Tv>> $items)
    : Map<Tk, Tv>;

  public function __toString(): string;

  public function items(): Iterable<Pair<Tk, Tv>>;
}

class MapIterator<Tk, Tv> implements KeyedIterator<Tk, Tv> {
  public function __construct();
  public function current(): Tv;
  public function key(): Tk;
  public function valid(): bool;
  public function next(): void;
  public function rewind(): void;
}

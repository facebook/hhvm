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

final class ImmVector<Tv> implements ConstVector<Tv> {
  /**
   * Create an empty ImmVector (if no parameters are passed) or create
   * an ImmVector from an Traversable (if one parameter is passed).
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
  public function containsKey(int $k): bool;

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
   * Returns an iterator that points to beginning of this ImmVector.
   */
  public function getIterator(): VectorIterator<Tv>;

  /**
   * Returns the index of the first element that matches the search value.
   * If no element matches the search value, this function returns -1.
   */
  public function linearSearch(Tv $search_value): int;

  /**
   * Returns an ImmVector containing the values from the specified array.
   */
  public static function fromArray(array $arr): ImmVector<Tv>;

  public static function fromItems(?Traversable<Tv> $items): ImmVector<Tv>;

  /**
   * Slice an ImmVector. This function provides the functionality of
   * array_slice() for ImmVectors. Note that this function returns
   * a new ImmVector and does not modify the original ImmVector.
   */
  public static function slice(ImmVector<Tv> $v, int $offset,
                               ?int $len = null): ImmVector<Tv>;

  public function __toString(): string;

  public function items(): Iterable<Tv>;

  /**
   * Returns a copy of this ImmVector.
   */
  public function toImmVector(): ImmVector<Tv>;

  /**
   * Returns a Vector containing the elements of this ImmVector.
   */
  public function toVector(): Vector<Tv>;

  /**
   * Returns a Map containing the elements of this ImmVector.
   * The keys are 0... count() - 1.
   */
  public function toMap(): Map<int, Tv>;
  public function toImmMap(): ImmMap<int, Tv>;

  /**
   * Returns a Set containing the elements of this ImmVector.
   */
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;

  public function lazy(): KeyedIterable<int, Tv>;
  public function values(): ImmVector<Tv>;
  public function keys(): ImmVector<int>;
  public function map<Tu>((function(Tv): Tu) $callback): ImmVector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $callback):
    ImmVector<Tu>;
  public function filter((function(Tv): bool) $callback): ImmVector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $callback):
    ImmVector<Tv>;
  public function zip<Tu>(Traversable<Tu> $iterable):
    ImmVector<Pair<Tv, Tu>>;
}

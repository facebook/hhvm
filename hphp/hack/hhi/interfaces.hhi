<?hh // partial
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
 * This file provides type information for some of PHP's predefined interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

interface Traversable<+Tv> {}

interface KeyedTraversable<+Tk, +Tv> extends Traversable<Tv> {}

interface Container<+Tv> extends Traversable<Tv> {}

interface KeyedContainer<+Tk, +Tv> extends Container<Tv>, KeyedTraversable<Tk, Tv> {}

interface Indexish<+Tk, +Tv> extends KeyedContainer<Tk, Tv> {}

interface Iterator<+Tv> extends Traversable<Tv> {
  public function current(): Tv;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;
}

interface AsyncIterator<+Tv> {
  public function next(): Awaitable<?(mixed, Tv)>;
}

interface AsyncKeyedIterator<+Tk, +Tv> extends AsyncIterator<Tv> {
  public function next(): Awaitable<?(Tk, Tv)>;
}

interface KeyedIterator<+Tk, +Tv> extends KeyedTraversable<Tk,Tv>, Iterator<Tv> {
  public function key(): Tk;
}

interface IteratorAggregate<+Tv> extends Traversable<Tv> {
  public function getIterator(): Iterator<Tv>;
}

interface Iterable<+Tv> extends IteratorAggregate<Tv> {
  public function getIterator(): Iterator<Tv>;
  public function toArray(): array;
  public function toValuesArray(): array;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe.
   * See #6853603. */
  public function toSet(): Set<Tv>;
  public function toImmSet(): ImmSet<Tv>;
  public function lazy(): Iterable<Tv>;
  public function values(): Iterable<Tv>;
  public function map<Tu>((function(Tv): Tu) $fn): Iterable<Tu>;
  public function filter((function(Tv): bool) $fn): Iterable<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv, Tu>>;
  public function take(int $n): Iterable<Tv>;
  public function takeWhile((function(Tv): bool) $fn): Iterable<Tv>;
  public function skip(int $n): Iterable<Tv>;
  public function skipWhile((function(Tv): bool) $fn): Iterable<Tv>;
  public function slice(int $start, int $len): Iterable<Tv>;
  public function concat<Tu super Tv>(
    Traversable<Tu> $traversable
  ): Iterable<Tu>;
  public function firstValue(): ?Tv;
  public function lastValue(): ?Tv;
}

interface KeyedIterable<Tk, +Tv> extends KeyedTraversable<Tk, Tv>, Iterable<Tv> {
  public function getIterator(): KeyedIterator<Tk, Tv>;
  public function toKeysArray(): array;
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toMap(): Map<Tk, Tv>;
  public function toImmMap(): ImmMap<Tk, Tv>;
  public function lazy(): KeyedIterable<Tk, Tv>;
  public function values(): Iterable<Tv>;
  public function keys(): Iterable<Tk>;
  public function map<Tu>((function(Tv): Tu) $fn): KeyedIterable<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn):
    KeyedIterable<Tk, Tu>;
  public function filter((function(Tv): bool) $fn): KeyedIterable<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $fn):
    KeyedIterable<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    KeyedIterable<Tk, Pair<Tv, Tu>>;
  public function take(int $n): KeyedIterable<Tk, Tv>;
  public function takeWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv>;
  public function skip(int $n): KeyedIterable<Tk, Tv>;
  public function skipWhile((function(Tv): bool) $fn): KeyedIterable<Tk, Tv>;
  public function slice(int $start, int $len): KeyedIterable<Tk, Tv>;
  public function concat<Tu super Tv>(
    Traversable<Tu> $traversable
  ): Iterable<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;
}

interface Serializable {
  public function serialize(): string;
  public function unserialize($serialized): void;
}

interface Countable {
  public function count(): int;
}

interface RecursiveIterator<Tv> extends Iterator<Tv> {
  public function getChildren(): this;
  public function hasChildren(): bool;
}

interface SeekableIterator<Tv> extends Iterator<Tv> {
  public function seek(int $position): void;
}

interface OuterIterator<Tv> extends Iterator<Tv> {
  public function getInnerIterator(): Iterator<Tv>;
}

interface ArrayAccess<Tk, Tv> {
  public function offsetExists(Tk $key): bool;
  public function offsetGet(Tk $key): Tv;
  public function offsetSet(Tk $key, Tv $val): void;
  public function offsetUnset(Tk $key): void;
}

interface Awaitable<+T> {
  public function getWaitHandle(): WaitHandle<T>;
}

/**
 * @see http://www.php.net/manual/en/jsonserializable.jsonserialize.php
 */
interface JsonSerializable {
  /**
   * Return data which can be serialized with json_encode.
   */
  public function jsonSerialize(): mixed;
}

/**
 * XHPChild is the base type of values that can be children of XHP elements.
 * Most primitive types implement XHPChild: string, int, float, and array.
 *
 * Classes that implement XHPChild must do so by implementing the XHPChildClass
 * subinterface.
 */
interface XHPChild {}

/**
 * Stringish is a type that matches strings as well as string-convertible
 * objects: that is, objects that provide the __toString method
 */
interface Stringish {
  public function __toString(): string;
}

/**
  * Classes that implement IMemoizeParam may be used as params on
  * <<__Memoize>> functions
  */
interface IMemoizeParam {
   /**
   * Serialize this object to a string that can be used as a
   * dictionary key to differentiate instances of this class.
   */
  public function getInstanceKey(): string;
}

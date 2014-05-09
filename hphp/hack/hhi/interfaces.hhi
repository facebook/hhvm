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

interface Traversable<Tv> {}

interface KeyedTraversable<Tk, Tv> extends Traversable<Tv> {}

interface Container<Tv> extends Traversable<Tv> {}

interface KeyedContainer<Tk, Tv> extends Container<Tv>, KeyedTraversable<Tk, Tv> {}

interface Indexish<Tk, Tv> extends KeyedContainer<Tk, Tv> {}

interface Iterator<Tv> extends Traversable<Tv> {
  public function current(): Tv;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;
}

interface KeyedIterator<Tk, Tv> extends KeyedTraversable<Tk,Tv>, Iterator<Tv> {
  public function key(): Tk;
}

interface IteratorAggregate<Tv> extends Traversable<Tv> {
  public function getIterator(): Iterator<Tv>;
}

interface Iterable<Tv> extends IteratorAggregate<Tv> {
  public function getIterator(): Iterator<Tv>;
  public function toArray(): array;
  public function toValuesArray(): array;
  public function toVector(): Vector<Tv>;
  public function toImmVector(): ImmVector<Tv>;
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
  public function concat(Traversable<Tv> $traversable): Iterable<Tv>;
  public function firstValue(): ?Tv;
  public function lastValue(): ?Tv;
}

interface KeyedIterable<Tk, Tv> extends KeyedTraversable<Tk, Tv>, Iterable<Tv> {
  public function getIterator(): KeyedIterator<Tk, Tv>;
  public function toKeysArray(): array;
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
  public function concat(Traversable<Tv> $traversable): Iterable<Tv>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;
}

interface Serializable {
  public function serialize(): string;
  public function unserialize(string $serialized): void;
}

interface Countable {
  public function count(): int;
}

interface ArrayAccess<Tk, Tv> {
  public function offsetExists(Tk $key): bool;
  public function offsetGet(Tk $key): Tv;
  public function offsetSet(Tk $key, Tv $val): this;
  public function offsetUnset(Tk $key): this;
}

interface Awaitable<T> {
  public function getWaitHandle(): WaitHandle<T>;
}

interface Continuation<Tv> extends KeyedIterator<int, Tv> {
  public function getOrigFuncName(): string;
  public function send($v): void;
  public function raise(Exception $e): void;
  public function getLabel(): int;
  public function update(int $label, Tv $value): void;
  public function num_args(): int;
  public function get_arg(int $index): mixed;
}

interface ConstCollection<Te> extends Countable {
  public function isEmpty(): bool;
  public function count(): int;
  public function items(): Iterable<Te>;
}

interface OutputCollection<Te> {
  public function add(Te $e): this;
  public function addAll(?Traversable<Te> $traversable): this;
}

interface Collection<Te> extends ConstCollection<Te>,
                                 OutputCollection<Te> {
  public function clear();
}

interface ConstSetAccess<Tm> {
  public function contains(Tm $m): bool;
}

interface SetAccess<Tm> extends ConstSetAccess<Tm> {
  public function remove(Tm $m): this;
}

interface ConstIndexAccess<Tk, Tv> {
  public function at(Tk $k): Tv;
  public function get(Tk $k): ?Tv;
  public function containsKey(Tk $k): bool;
}

interface IndexAccess<Tk, Tv> extends ConstIndexAccess<Tk, Tv> {
  public function set(Tk $k, Tv $v): this;
  public function setAll(?KeyedTraversable<Tk, Tv> $traversable): this;
  public function removeKey(Tk $k): this;
}

interface ConstMapAccess<Tk, Tv> extends ConstSetAccess<Tk>,
                                         ConstIndexAccess<Tk, Tv> {
}

interface MapAccess<Tk, Tv> extends ConstMapAccess<Tk, Tv>,
                                    SetAccess<Tk>,
                                    IndexAccess<Tk, Tv> {
}

interface ConstVector<Tv> extends ConstCollection<Tv>,
                                  ConstIndexAccess<int, Tv>,
                                  KeyedIterable<int, Tv>,
                                  Indexish<int, Tv> {
  public function values(): ConstVector<Tv>;
  public function keys(): ConstVector<int>;
  public function map<Tu>((function(Tv): Tu) $fn): ConstVector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $fn):
    ConstVector<Tu>;
  public function filter((function(Tv): bool) $fn): ConstVector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $fn):
    ConstVector<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): ConstVector<Pair<Tv, Tu>>;
  public function take(int $n): ConstVector<Tv>;
  public function takeWhile((function(Tv): bool) $fn): ConstVector<Tv>;
  public function skip(int $n): ConstVector<Tv>;
  public function skipWhile((function(Tv): bool) $fn): ConstVector<Tv>;
  public function slice(int $start, int $len): ConstVector<Tv>;
  public function concat(Traversable<Tv> $traversable): ConstVector<Tv>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?int;
  public function lastValue(): ?Tv;
  public function lastKey(): ?int;
}

interface MutableVector<Tv> extends ConstVector<Tv>,
                                    Collection<Tv>,
                                    IndexAccess<int, Tv> {
  public function values(): MutableVector<Tv>;
  public function keys(): MutableVector<int>;
  public function map<Tu>((function(Tv): Tu) $fn): MutableVector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $fn):
    MutableVector<Tu>;
  public function filter((function(Tv): bool) $fn): MutableVector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $fn):
    MutableVector<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    MutableVector<Pair<Tv, Tu>>;
  public function take(int $n): MutableVector<Tv>;
  public function takeWhile((function(Tv): bool) $fn): MutableVector<Tv>;
  public function skip(int $n): MutableVector<Tv>;
  public function skipWhile((function(Tv): bool) $fn): MutableVector<Tv>;
  public function slice(int $start, int $len): MutableVector<Tv>;
  public function concat(Traversable<Tv> $traversable): MutableVector<Tv>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?int;
  public function lastValue(): ?Tv;
  public function lastKey(): ?int;
}

interface ConstMap<Tk, Tv> extends ConstCollection<Pair<Tk, Tv>>,
                                   ConstMapAccess<Tk, Tv>,
                                   KeyedIterable<Tk, Tv>,
                                   Indexish<Tk, Tv> {
  public function values(): ConstVector<Tv>;
  public function keys(): ConstVector<Tk>;
  public function map<Tu>((function(Tv): Tu) $fn): ConstMap<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn):
    ConstMap<Tk, Tu>;
  public function filter((function(Tv): bool) $fn): ConstMap<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $fn):
    ConstMap<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    ConstMap<Tk, Pair<Tv, Tu>>;
  public function take(int $n): ConstMap<Tk, Tv>;
  public function takeWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv>;
  public function skip(int $n): ConstMap<Tk, Tv>;
  public function skipWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv>;
  public function slice(int $start, int $len): ConstMap<Tk, Tv>;
  public function concat(Traversable<Tv> $traversable): ConstVector<Tv>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;
}

interface MutableMap<Tk, Tv> extends ConstMap<Tk, Tv>,
                                     Collection<Pair<Tk, Tv>>,
                                     MapAccess<Tk, Tv> {
  public function values(): MutableVector<Tv>;
  public function keys(): MutableVector<Tk>;
  public function map<Tu>((function(Tv): Tu) $fn): MutableMap<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn):
    MutableMap<Tk, Tu>;
  public function filter((function(Tv): bool) $fn): MutableMap<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $fn):
    MutableMap<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    MutableMap<Tk, Pair<Tv, Tu>>;
  public function take(int $n): MutableMap<Tk, Tv>;
  public function takeWhile((function(Tv): bool) $fn): MutableMap<Tk, Tv>;
  public function skip(int $n): MutableMap<Tk, Tv>;
  public function skipWhile((function(Tv): bool) $fn): MutableMap<Tk, Tv>;
  public function slice(int $start, int $len): MutableMap<Tk, Tv>;
  public function concat(Traversable<Tv> $traversable): MutableVector<Tv>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;
}

interface ConstSet<Tv> extends ConstCollection<Tv>,
                               ConstSetAccess<Tv>,
                               Container<Tv>,
                               Iterable<Tv> {
  public function values(): ConstVector<Tv>;
  public function map<Tu>((function(Tv): Tu) $fn): ConstSet<Tu>;
  public function filter((function(Tv): bool) $fn): ConstSet<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): ConstSet<Pair<Tv, Tu>>;
  public function take(int $n): ConstSet<Tv>;
  public function takeWhile((function(Tv): bool) $fn): ConstSet<Tv>;
  public function skip(int $n): ConstSet<Tv>;
  public function skipWhile((function(Tv): bool) $fn): ConstSet<Tv>;
  public function slice(int $start, int $len): ConstSet<Tv>;
  public function concat(Traversable<Tv> $traversable): ConstVector<Tv>;
  public function firstValue(): ?Tv;
  public function lastValue(): ?Tv;
}

interface MutableSet<Tv> extends ConstSet<Tv>,
                                 Collection<Tv>,
                                 SetAccess<Tv> {
  public function values(): MutableVector<Tv>;
  public function map<Tu>((function(Tv): Tu) $fn): MutableSet<Tu>;
  public function filter((function(Tv): bool) $fn): MutableSet<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    MutableSet<Pair<Tv, Tu>>;
  public function take(int $n): MutableSet<Tv>;
  public function takeWhile((function(Tv): bool) $fn): MutableSet<Tv>;
  public function skip(int $n): MutableSet<Tv>;
  public function skipWhile((function(Tv): bool) $fn): MutableSet<Tv>;
  public function slice(int $start, int $len): MutableSet<Tv>;
  public function concat(Traversable<Tv> $traversable): MutableVector<Tv>;
  public function firstValue(): ?Tv;
  public function lastValue(): ?Tv;
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

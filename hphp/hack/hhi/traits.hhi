<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined interfaces
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

trait StrictIterable<+Tv> implements Iterable<Tv> {

  <<__PHPStdLib>>
  public function toArray(): array;

  public function toValuesArray(): varray<Tv>;

  public function toImmVector(): ImmVector<Tv>;

  public function toImmSet(): ImmSet<Tv> where Tv as arraykey;

  public function lazy(): Iterable<Tv>;

  public function values(): Iterable<Tv>;

  public function map<Tu>((function(Tv): Tu) $callback): Iterable<Tu>;

  public function filter((function(Tv): bool) $callback): Iterable<Tv>;

  public function zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv,Tu>>;

  public function take(int $n): Iterable<Tv>;

  public function takeWhile((function(Tv): bool) $n): Iterable<Tv>;

  public function skip(int $n): Iterable<Tv>;

  public function skipWhile((function(Tv): bool) $n): Iterable<Tv>;

  public function slice(int $start, int $len): Iterable<Tv>;

  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu>;

  public function firstValue(): ?Tv;

  public function lastValue(): ?Tv;

}


trait StrictKeyedIterable<Tk,+Tv> implements KeyedIterable<Tk,Tv> {
  <<__PHPStdLib>>
  public function toArray(): array;

  public function toValuesArray(): varray<Tv>;

  public function toKeysArray(): varray<Tk>;

  public function toImmVector(): ImmVector<Tv>;

  public function toImmMap(): ImmMap<Tk, Tv> where Tk as arraykey;

  public function toImmSet(): ImmSet<Tv> where Tv as arraykey;

  public function lazy(): KeyedIterable<Tk,Tv>;

  public function values(): Iterable<Tv>;

  public function keys(): Iterable<Tk>;

  public function map<Tu>((function(Tv): Tu) $callback): KeyedIterable<Tk,Tu>;

  public function mapWithKey<Tu>(
    (function(Tk,Tv): Tu) $callback
  ): KeyedIterable<Tk,Tu>;

  public function filter((function(Tv): bool) $callback): KeyedIterable<Tk,Tv>;

  public function filterWithKey(
    (function(Tk,Tv): bool) $callback
  ): KeyedIterable<Tk,Tv>;

  public function zip<Tu>(
    Traversable<Tu> $traversable
  ): KeyedIterable<Tk,Pair<Tv,Tu>>;

  public function take(int $n): KeyedIterable<Tk, Tv>;

  public function takeWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv>;

  public function skip(int $n): KeyedIterable<Tk, Tv>;

  public function skipWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv>;

  public function slice(int $start, int $len): KeyedIterable<Tk, Tv>;

  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu>;

  public function firstValue(): ?Tv;

  public function firstKey(): ?Tk;

  public function lastValue(): ?Tv;

  public function lastKey(): ?Tk;

}


trait LazyIterable<+Tv> implements Iterable<Tv> {
  <<__PHPStdLib>>
  public function toArray(): array;

  public function toValuesArray(): varray<Tv>;

  public function toImmVector(): ImmVector<Tv>;

  public function toImmSet(): ImmSet<Tv> where Tv as arraykey;

  public function lazy(): Iterable<Tv>;

  public function values(): Iterable<Tv>;

  public function map<Tu>((function(Tv): Tu) $callback): Iterable<Tu>;

  public function filter((function(Tv): bool) $callback): Iterable<Tv>;

  public function zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv,Tu>>;

  public function take(int $n): Iterable<Tv>;

  public function takeWhile((function(Tv): bool) $n): Iterable<Tv>;

  public function skip(int $n): Iterable<Tv>;

  public function skipWhile((function(Tv): bool) $n): Iterable<Tv>;

  public function slice(int $start, int $len): Iterable<Tv>;

  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu>;

  public function firstValue(): ?Tv;

  public function lastValue(): ?Tv;

}


trait LazyKeyedIterable<Tk,+Tv> implements KeyedIterable<Tk,Tv> {
  <<__PHPStdLib>>
  public function toArray(): array;

  public function toValuesArray(): varray<Tv>;

  public function toKeysArray(): varray<Tk>;

  public function toImmVector(): ImmVector<Tv>;

  public function toImmMap(): ImmMap<Tk, Tv> where Tk as arraykey;

  public function toImmSet(): ImmSet<Tv> where Tv as arraykey;

  public function lazy(): KeyedIterable<Tk,Tv>;

  public function values(): Iterable<Tv>;

  public function keys(): Iterable<Tk>;

  public function map<Tu>((function(Tv): Tu) $callback): KeyedIterable<Tk,Tu>;

  public function mapWithKey<Tu>(
    (function(Tk,Tv): Tu) $callback
  ): KeyedIterable<Tk,Tu>;

  public function filter((function(Tv): bool) $callback): KeyedIterable<Tk,Tv>;

  public function filterWithKey(
    (function(Tk,Tv): bool) $callback
  ): KeyedIterable<Tk,Tv>;

  public function zip<Tu>(
    Traversable<Tu> $traversable
  ): KeyedIterable<Tk,Pair<Tv,Tu>>;

  public function take(int $n): KeyedIterable<Tk, Tv>;

  public function takeWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv>;

  public function skip(int $n): KeyedIterable<Tk, Tv>;

  public function skipWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv>;

  public function slice(int $start, int $len): KeyedIterable<Tk, Tv>;

  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu>;

  public function firstValue(): ?Tv;

  public function firstKey(): ?Tk;

  public function lastValue(): ?Tv;

  public function lastKey(): ?Tk;

}

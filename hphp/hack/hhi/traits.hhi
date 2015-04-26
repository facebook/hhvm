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

trait StrictIterable<+Tv> implements Iterable<Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toSet(): Set<Tv> {
    return Set {};
  }
  public function toImmSet(): ImmSet<Tv> {
    return ImmSet {};
  }
  public function lazy(): Iterable<Tv> {
    return Vector {};
  }
  public function values(): Iterable<Tv> {
    return Vector {};
  }
  public function map<Tu>((function(Tv): Tu) $callback): Iterable<Tu> {
    return Vector {};
  }
  public function filter((function(Tv): bool) $callback): Iterable<Tv> {
    return Vector {};
  }
  public function zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv,Tu>> {
    return Vector {};
  }
  public function take(int $n): Iterable<Tv> {
    return Vector {};
  }
  public function takeWhile((function(Tv): bool) $n): Iterable<Tv> {
    return Vector {};
  }
  public function skip(int $n): Iterable<Tv> {
    return Vector {};
  }
  public function skipWhile((function(Tv): bool) $n): Iterable<Tv> {
    return Vector {};
  }
  public function slice(int $start, int $len): Iterable<Tv> {
    return Vector {};
  }
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> {
    return Vector {};
  }
  public function firstValue(): ?Tv {
    return null;
  }
  public function lastValue(): ?Tv {
    return null;
  }
}

trait StrictKeyedIterable<Tk,+Tv> implements KeyedIterable<Tk,Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  public function toKeysArray(): array {
    return array();
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toMap(): Map<Tk, Tv> {
    return Map {};
  }
  public function toImmMap(): ImmMap<Tk, Tv> {
    return ImmMap {};
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toSet(): Set<Tv> {
    return Set {};
  }
  public function toImmSet(): ImmSet<Tv> {
    return ImmSet {};
  }
  public function lazy(): KeyedIterable<Tk,Tv> {
    return Map {};
  }
  public function values(): Iterable<Tv> {
    return Vector {};
  }
  public function keys(): Iterable<Tk> {
    return Vector {};
  }
  public function map<Tu>((function(Tv): Tu) $callback): KeyedIterable<Tk,Tu> {
    return Map {};
  }
  public function mapWithKey<Tu>(
    (function(Tk,Tv): Tu) $callback
  ): KeyedIterable<Tk,Tu> {
    return Map {};
  }
  public function filter((function(Tv): bool) $callback): KeyedIterable<Tk,Tv> {
    return Map {};
  }
  public function filterWithKey(
    (function(Tk,Tv): bool) $callback
  ): KeyedIterable<Tk,Tv> {
    return Map {};
  }
  public function zip<Tu>(
    Traversable<Tu> $traversable
  ): KeyedIterable<Tk,Pair<Tv,Tu>> {
    return Map {};
  }
  public function take(int $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function takeWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function skip(int $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function skipWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function slice(int $start, int $len): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> {
    return Map {};
  }
  public function firstValue(): ?Tv {
    return null;
  }
  public function firstKey(): ?Tk {
    return null;
  }
  public function lastValue(): ?Tv {
    return null;
  }
  public function lastKey(): ?Tk {
    return null;
  }
}

trait LazyIterable<+Tv> implements Iterable<Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toSet(): Set<Tv> {
    return Set {};
  }
  public function toImmSet(): ImmSet<Tv> {
    return ImmSet {};
  }
  public function lazy(): Iterable<Tv> {
    return Vector {};
  }
  public function values(): Iterable<Tv> {
    return Vector {};
  }
  public function map<Tu>((function(Tv): Tu) $callback): Iterable<Tu> {
    return Vector {};
  }
  public function filter((function(Tv): bool) $callback): Iterable<Tv> {
    return Vector {};
  }
  public function zip<Tu>(Traversable<Tu> $traversable): Iterable<Pair<Tv,Tu>> {
    return Vector {};
  }
  public function take(int $n): Iterable<Tv> {
    return Vector {};
  }
  public function takeWhile((function(Tv): bool) $n): Iterable<Tv> {
    return Vector {};
  }
  public function skip(int $n): Iterable<Tv> {
    return Vector {};
  }
  public function skipWhile((function(Tv): bool) $n): Iterable<Tv> {
    return Vector {};
  }
  public function slice(int $start, int $len): Iterable<Tv> {
    return Vector {};
  }
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tu> {
    return Vector {};
  }
  public function firstValue(): ?Tv {
    return null;
  }
  public function lastValue(): ?Tv {
    return null;
  }
}

trait LazyKeyedIterable<Tk,+Tv> implements KeyedIterable<Tk,Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  public function toKeysArray(): array {
    return array();
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toMap(): Map<Tk, Tv> {
    return Map {};
  }
  public function toImmMap(): ImmMap<Tk, Tv> {
    return ImmMap {};
  }
  /* HH_FIXME[4120]: While this violates our variance annotations, we are
   * returning a copy of the underlying collection, so it is actually safe
   * See #6853603. */
  public function toSet(): Set<Tv> {
    return Set {};
  }
  public function toImmSet(): ImmSet<Tv> {
    return ImmSet {};
  }
  public function lazy(): KeyedIterable<Tk,Tv> {
    return Map {};
  }
  public function values(): Iterable<Tv> {
    return Vector {};
  }
  public function keys(): Iterable<Tk> {
    return Vector {};
  }
  public function map<Tu>((function(Tv): Tu) $callback): KeyedIterable<Tk,Tu> {
    return Map {};
  }
  public function mapWithKey<Tu>(
    (function(Tk,Tv): Tu) $callback
  ): KeyedIterable<Tk,Tu> {
    return Map {};
  }
  public function filter((function(Tv): bool) $callback): KeyedIterable<Tk,Tv> {
    return Map {};
  }
  public function filterWithKey(
    (function(Tk,Tv): bool) $callback
  ): KeyedIterable<Tk,Tv> {
    return Map {};
  }
  public function zip<Tu>(
    Traversable<Tu> $traversable
  ): KeyedIterable<Tk,Pair<Tv,Tu>> {
    return Map {};
  }
  public function take(int $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function takeWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function skip(int $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function skipWhile((function(Tv): bool) $n): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function slice(int $start, int $len): KeyedIterable<Tk, Tv> {
    return Map {};
  }
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): Iterable<Tv> {
    return Map {};
  }
  public function firstValue(): ?Tv {
    return null;
  }
  public function firstKey(): ?Tk {
    return null;
  }
  public function lastValue(): ?Tv {
    return null;
  }
  public function lastKey(): ?Tk {
    return null;
  }
}

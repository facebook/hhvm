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

trait StrictIterable<Tv> implements Iterable<Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
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
  public function zip<Tu>(Traversable<Tu> $iterable): Iterable<Pair<Tv,Tu>> {
    return Vector {};
  }
}

trait StrictKeyedIterable<Tk,Tv> implements KeyedIterable<Tk,Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  public function toKeysArray(): array {
    return array();
  }
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
  public function toMap(): Map<Tk, Tv> {
    return Map {};
  }
  public function toImmMap(): ImmMap<Tk, Tv> {
    return ImmMap {};
  }
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
    Traversable<Tu> $iterable
  ): KeyedIterable<Tk,Pair<Tv,Tu>> {
    return Map {};
  }
}

trait LazyIterable<Tv> implements Iterable<Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
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
  public function zip<Tu>(Traversable<Tu> $iterable): Iterable<Pair<Tv,Tu>> {
    return Vector {};
  }
}

trait LazyKeyedIterable<Tk,Tv> implements KeyedIterable<Tk,Tv> {
  public function toArray(): array {
    return array();
  }
  public function toValuesArray(): array {
    return array();
  }
  public function toKeysArray(): array {
    return array();
  }
  public function toVector(): Vector<Tv> {
    return Vector {};
  }
  public function toImmVector(): ImmVector<Tv> {
    return ImmVector {};
  }
  public function toMap(): Map<Tk, Tv> {
    return Map {};
  }
  public function toImmMap(): ImmMap<Tk, Tv> {
    return ImmMap {};
  }
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
    Traversable<Tu> $iterable
  ): KeyedIterable<Tk,Pair<Tv,Tu>> {
    return Map {};
  }
}


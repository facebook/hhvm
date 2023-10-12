<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

/**
 * Handy functions to create iterators.
 *
 * Not using generators to be compatible with code that is explicitly setting
 * Hack.Lang.AutoprimeGenerators to false.
 */
abstract final class HackLibTestTraversables {

  // For testing functions that accept Traversables
  public static function getIterator<T>(Traversable<T> $ary): \HH\Iterator<T> {
    $dict = dict[];
    $i = 0;
    foreach ($ary as $v) {
      $dict[$i] = $v;
      $i++;
    }
    return new HackLibTestForwardOnlyIterator($dict);
  }

  // For testing functions that accept KeyedTraversables
  public static function getKeyedIterator<Tk as arraykey, Tv>(
    KeyedTraversable<Tk, Tv> $ary,
  ): \HH\KeyedIterator<Tk, Tv> {
    return new HackLibTestForwardOnlyIterator(dict($ary));
  }
}

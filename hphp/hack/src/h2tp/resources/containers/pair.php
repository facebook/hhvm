<?php
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

namespace HH {

  require_once(__DIR__.SEP.'interfaces.php');
  require_once(__DIR__.SEP.'helper_traits'.SEP.'hacklib_immVectorLike.php');
  require_once(__DIR__.SEP.'pairIterator.php');

  final class Pair implements \ConstVector, \ArrayAccess, \Stringish {
    use HACKLIB_ImmVectorLike;

    private static $pCreator;

    public function __construct() {
      throw new \InvalidOperationException(
        'Pairs cannot be created using the new operator');
    }

    /**
     * used by HACKLIB_iteratable.
     * returns an iterator of the appropriate type
     */
    protected function hacklib_createNewIterator() {
      return new \PairIterator();
    }

    public function __toString() {
      return "Pair";
    }

    public function immutable() {
      return $this;
    }

    private static function hacklib_new_instance() {
      if (!self::$pCreator) {
        self::$pCreator = new \ReflectionClass('\HH\Pair');
      }
      return self::$pCreator->newInstanceWithoutConstructor();
    }

    public static function hacklib_new($p1, $p2) {
      $p = static::hacklib_new_instance();
      $p->hacklib_init_t(array($p1, $p2));
      return $p;
    }
  }
}

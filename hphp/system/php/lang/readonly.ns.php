<?hh
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

namespace HH\Readonly {

  /**
   * Converts a readonly value type into a mutable one.
   * Value types include numerics, strings, bools, null and Hack arrays of value
   * types.
   */
  <<__IgnoreReadonlyLocalErrors>>
  function as_mut<T>(readonly T $x)[]: T {
    if (is_value_type(readonly $x)) {
      return $x;
    }
    throw new \InvalidArgumentException(__FUNCTION__.' expects a value type');
  }

  function is_value_type(readonly mixed $x)[]: bool {
    if ($x is string || $x is int || $x is float || $x is bool || $x is null) {
      return true;
    }
    if ($x is \HH\AnyArray) {
      foreach ($x as $v) {
        if (!is_value_type($v)) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  /**
   * Converts a readonly value to into a mutable one but omits validation for the
   * purposes of performance.
   * This function takes advantage of non-enforcement of readonly is SystemLib
   * and is not safe.
   */
  <<__IgnoreReadonlyLocalErrors>>
  function as_mut_without_validation<T>(readonly T $x)[]: T {
    return $x;
  }

  /*
  Readonly versions of Shape helper functions
  Shapes::toArray and Shapes::at are copied because of their simplicity, but most other functions
  are ported directly to their counterparts in HH\Shapes (see ext_shapes.php)
  */
  abstract final class Shapes {
    <<__IsFoldable, __IgnoreReadonlyLocalErrors>>
    public static function idx(
      readonly ?darray $shape,
      readonly arraykey $index,
      readonly mixed $default = null,
    )[]: readonly mixed {
      return \HH\Shapes::idx($shape, $index, $default);
    }

    public static function at(
      readonly darray $shape,
      readonly arraykey $index,
    )[]: readonly mixed {
      return $shape[$index];
    }

    public static function toArray(readonly darray $shape)[]: readonly darray {
      return $shape;
    }

    <<__IgnoreReadonlyLocalErrors>>
    public static function toDict(readonly darray $shape)[]: readonly dict {
      return \HH\Shapes::toDict($shape);
    }
  }

}

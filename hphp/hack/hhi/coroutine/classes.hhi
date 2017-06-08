<?hh // strict
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

class SuspendedCoroutineResult<T> implements CoroutineResult<T> {

  private static ?SuspendedCoroutineResult<T> $instance = null;

  private function __construct() {}

  public function getResult(): T {
    throw new Exception(
      "getResult() was called on a suspended CoroutineResult.");
  }

  public function isSuspended(): bool {
    return true;
  }

  public static function create(): CoroutineResult<T> {
    if (self::$instance === null) {
      self::$instance = new SuspendedCoroutineResult();
    }
    return self::$instance;
  }
}

/**
 * Represents a unit type for the purposes of internal use with coroutines.
 *
 * It is difficult to deal with coroutines that are either void-returning, or if
 * the point at which the coroutine suspended is not expecting a value, because
 * it is tricky to use void as a type parameter.
 *
 * Rather than having multiple methods like "resume_with_no_value()", we simply
 * define a unit singleton and use it where void is intended.
 */
final class CoroutineUnit {

  private static ?CoroutineUnit $unit = null;

  private function __construct() {}

  public static function create(): CoroutineUnit {
    if (self::$unit === null) {
      self::$unit = new CoroutineUnit();
    }
    return self::$unit;
  }
}

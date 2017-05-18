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

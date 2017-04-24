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

/**
 * Represents the remainder of a suspended coroutine.
 *
 * The continuation is executed when the needed result is available. The
 * continuation is waiting on either a value, or, if no value is available,
 * the exception which prevented it from being available.
 */
interface CoroutineContinuation<-T> {
  public function resume(T $value): void;
  public function resumeWithException(Exception $ex): void;
}

/**
 * The synchronous result obtained from calling a coroutine function.
 *
 * A coroutine function call has four possible outcomes:
 *
 *   1. It runs forever.
 *   2. It returns a value.
 *   3. It throws an exception.
 *   4. It suspends.
 *
 * We disambiguate 2. from 4. by wrapping all returns in a wrapper class, and
 * having a special "suspended" value, of a singleton type.
 */
interface CoroutineResult<+T> {
  public function getResult(): T;
  public function isSuspended(): bool;
}

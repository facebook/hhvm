<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * Object of this type is returned by `__FUNCTION_CREDENTIAL__`
 * but you almost certainly should NOT be attempting to construct this object
 */
final class FunctionCredential {
  private function __construct() {}

  public function getClassName()[]: ?string;

  public function getFunctionName()[]: string;

  public function getFilename()[]: string;

  /**
   * Packs this FunctionCredential into a signed string representation.
   * Intention is to pass FunctionCredential to another thread in the same
   * process. DO NOT USE IF PASSING TO ANOTHER HHVM PROCESS OR PLANNING TO UNPACK
   * MORE THAN 5 MINUTES INTO THE FUTURE.
   *
   * The packed string contains the class name, function name, and an
   * expiration timestamp, all signed with a per-process authentication key.
   * The packed credential expires 5 minutes after creation.
   *
   * Use unpack() to reconstruct a FunctionCredential from the packed string.
   */
  public function pack()[]: string;

  /**
   * Unpacks a FunctionCredential from its signed string representation.
   *
   * Only succeeds if you unpack a value that was produced by pack()
   * in the exact same hhvm process within the last 5 minutes. Will not work
   * across restarts, if passed to another hhvm process (e.g. an async job or
   * child web request), or if more than 5 minutes have elapsed since
   * packing.
   *
   * @throws InvalidArgumentException if the packed string is invalid, expired,
   *         was packed in a different process, or references a non-existent
   *         function/class.
   */
  public static function unpack(string $packed)[]: FunctionCredential;

  public function __debugInfo(): darray<string, ?string>;
}

<?hh // partial
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined functions
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace HH\Rx;

interface Exception {
 require extends \Exception;
 <<__Rx, __MaybeMutable>>
 public function getMessage(): string;
 <<__Rx, __MaybeMutable>>
 public function getCode(): int;
}

<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

final class WeakRef<T> {
  public function __construct(T $object);
  public function acquire(): bool;
  public function get(): ?T;
  public function release(): bool;
  public function valid(): bool;
}

<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

final class WeakRef<T> {
  public function __construct(T $object)[];
  public function acquire()[write_props]: bool;
  public function get()[]: ?T;
  public function release()[write_props]: bool;
  public function valid()[]: bool;
}

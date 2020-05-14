<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

final class EmptyIterator implements Iterator<nothing> {
  /* Both current() and key() on an EmptyIterator throw unconditionally */
  public function current(): nothing;
  public function key(): nothing;
  public function next(): void;
  public function rewind(): void;
  public function valid(): bool;

}

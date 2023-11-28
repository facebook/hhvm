<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class RecursiveRegexIterator<Tv>
  extends RegexIterator<Tv>
  implements RecursiveIterator<~Tv> {

  // Methods
  public function __construct(
    RecursiveIterator<Tv> $iterator,
    string $regex,
    int $mode = \RecursiveRegexIterator::MATCH,
    int $flags = 0,
    int $preg_flags = 0,
  );
  public function accept(): bool;
  public function getChildren(): this;
  public function hasChildren(): bool;
}

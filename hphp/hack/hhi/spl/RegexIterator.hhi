<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class RegexIterator<Tv> extends FilterIterator<Tv> {

  // Constants
  const int MATCH;
  const int GET_MATCH;
  const int ALL_MATCHES;
  const int SPLIT;
  const int REPLACE;
  const int USE_KEY;
  const int INVERT_MATCH;

  // Methods
  public function __construct(
    Iterator<Tv> $iterator,
    string $regex,
    int $mode = \RegexIterator::MATCH,
    int $flags = 0,
    int $preg_flags = 0,
  );
  public function accept(): bool;
  public function getRegex();
  public function getMode(): int;
  public function getFlags(): int;
  public function getPregFlags(): int;
  public function setMode(int $mode): void;
  public function setFlags(int $flags): void;
  public function setPregFlags(int $preg_flags): void;
}

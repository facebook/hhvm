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

class RegexIterator<Tv> extends FilterIterator<Tv> {

  // Constants
  const MATCH = 0;
  const GET_MATCH = 1;
  const ALL_MATCHES = 2;
  const SPLIT = 3;
  const REPLACE = 4;
  const USE_KEY = 1;
  const INVERT_MATCH = 2;

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

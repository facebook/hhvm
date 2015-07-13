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

class DatePeriod implements Iterator<DateTime> {

  // Constants
  const EXCLUDE_START_DATE = 1;

  // Methods
  public function __construct(
    /* DateTimeInterface */ $start, // date string converts
    ?DateInterval $interval = null,
    /* ?DateTimeInterface */ $end = null, // date string converts
    int $options = 0,
  );
  public function current(): DateTime;
  public function rewind(): void;
  public function key();
  public function next();
  public function valid(): bool;
}

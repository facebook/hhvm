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

class DateTimeImmutable implements DateTimeInterface {

  private DateTime $data;

  // Methods
  public function __construct(
    string $time = 'now',
    ?DateTimeZone $timezone = null,
  );
  public function add(DateInterval $interval): this;
  public function modify(string $modify): this;
  public function setDate(int $year, int $month, int $day): this;
  public function setISODate(
    int $year,
    int $week,
    int $day = 1,
  ): this;
  public function setTime(
    int $hour,
    int $minute,
    int $second = 0,
  ): this;
  public function setTimestamp(int $unixtimestamp): this;
  public function setTimezone(DateTimeZone $timezone): this;
  public function sub(DateInterval $interval): this;
  public function diff(DateTimeInterface $datetime2, $absolute = false);
  public function format($format);
  public function getOffset();
  public function getTimestamp();
  public function getTimezone();
  public static function createFromFormat(
    string $format,
    string $time,
    ?DateTimeZone $timezone = null,
  ): mixed; // false | this
  public static function getLastErrors(): array;
  public function __clone();
}

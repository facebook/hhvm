<?hh // decl    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const DATE_ATOM    = "Y-m-d\\TH:i:sP";
const DATE_COOKIE  = "l, d-M-y H:i:s T";
const DATE_ISO8601 = "Y-m-d\\TH:i:sO";
const DATE_RFC1036 = "D, d M y H:i:s O";
const DATE_RFC1123 = "D, d M Y H:i:s O";
const DATE_RFC2822 = "D, d M Y H:i:s O";
const DATE_RFC3339 = "Y-m-d\\TH:i:sP";
const DATE_RFC822  = "D, d M y H:i:s O";
const DATE_RFC850  = "l, d-M-y H:i:s T";
const DATE_RSS     = "D, d M Y H:i:s O";
const DATE_W3C     = "Y-m-d\\TH:i:sP";

const DAY_1 = 131079;
const DAY_2 = 131080;
const DAY_3 = 131081;
const DAY_4 = 131082;
const DAY_5 = 131083;
const DAY_6 = 131084;
const DAY_7 = 131085;

function checkdate($month, $day, $year) { }
function date_add($datetime, $interval) { }
function date_create_from_format($format, $time, $timezone = null) { }
function date_create($time = null, $timezone = null) { }
function date_date_set($object, $year, $month, $day) { }
function date_default_timezone_get() { }
function date_default_timezone_set($name) { }
function date_diff($datetime, $datetime2, $absolute = false) { }
function date_format($object, $format) { }
function date_get_last_errors() { }
function date_interval_create_from_date_string($time) { }
function date_interval_format($interval, $format_spec) { }
function date_isodate_set($object, $year, $week, $day = 1) { }
function date_modify($object, $modify) { }
function date_offset_get($object) { }
function date_parse($date) { }
function date_sub($datetime, $interval) { }
function date_sun_info($ts, $latitude, $longitude) { }
function date_sunrise($timestamp, $format = 0, $latitude = 0.0, $longitude = 0.0, $zenith = 0.0, $gmt_offset = 99999.0) { }
function date_sunset($timestamp, $format = 0, $latitude = 0.0, $longitude = 0.0, $zenith = 0.0, $gmt_offset = 99999.0) { }
function date_time_set($object, $hour, $minute, $second = 0) { }
function date_timestamp_get($datetime) { }
function date_timestamp_set($datetime, $timestamp) { }
function date_timezone_get($object) { }
function date_timezone_set($object, $timezone) { }
function date($format, $timestamp = null)/*: string*/ { }
function getdate($timestamp = null) { }
function gettimeofday($return_float = false) { }
function gmdate($format, $timestamp = null) { }
function gmmktime($hour = PHP_INT_MAX, $minute = PHP_INT_MAX, $second = PHP_INT_MAX, $month = PHP_INT_MAX, $day = PHP_INT_MAX, $year = PHP_INT_MAX) { }
function gmstrftime($format, $timestamp = null) { }
function idate($format, $timestamp = null) { }
function localtime($timestamp = null, $is_associative = false) { }
function microtime($get_as_float = false) { }
function mktime($hour = PHP_INT_MAX, $minute = PHP_INT_MAX, $second = PHP_INT_MAX, $month = PHP_INT_MAX, $day = PHP_INT_MAX, $year = PHP_INT_MAX) { }
function strftime($format, $timestamp = null) { }
function strptime($date, $format) { }
function strtotime($input, $timestamp = null) { }
function time(): int { }
function timezone_abbreviations_list() { }
function timezone_identifiers_list() { }
function timezone_location_get($timezone) { }
function timezone_name_from_abbr($abbr, $gmtoffset = -1, $isdst = true) { }
function timezone_name_get($object) { }
function timezone_offset_get($object, $dt) { }
function timezone_open($timezone) { }
function timezone_transitions_get(DateTimeZone $object,
                                  int $timestamp_begin = PHP_INT_MIN,
                                  int $timestamp_end = PHP_INT_MAX) { }
function timezone_version_get() { }

interface DateTimeInterface {
  public function diff(DateTimeInterface $datetime2, bool $absolute = false) : DateInterval;
  public function format(string $format) : string;
  public function getOffset() : int;
  public function getTimestamp() : int;
  public function getTimezone() : DateTimeZone;
  public function __wakeup() : void;
}
class DateTime implements DateTimeInterface {
  const ATOM = '';
  const COOKIE = '';
  const ISO8601 = '';
  const RFC822 = '';
  const RFC850 = '';
  const RFC1036 = '';
  const RFC1123 = '';
  const RFC2822 = '';
  const RFC3339 = '';
  const RSS = '';
  const W3C = '';
  public function add(DateInterval $interval) { }
  public function __construct(string $time = "now", ?DateTimeZone $timezone = null) { }
  public static function createFromFormat(string $format, string $time, ?DateTimeZone $timezone = null) { }
  public function diff(DateTimeInterface $datetime2, bool $absolute = false) : DateInterval { }
  public function format(string $format) : string { }
  public static function getLastErrors() { }
  public function getOffset() : int { }
  public function getTimestamp() :int { }
  public function getTimezone() : DateTimeZone { }
  public function modify(string $modify) : DateTime { }
  public function setDate(int $year, int $month, int $day) : DateTime { }
  public function setISODate(int $year, int $week, int $day = 1) : DateTime { }
  public function setTime(int $hour, int $minute, int $second = 0) : DateTime { }
  public function setTimestamp(int $unixtimestamp) :DateTime { }
  public function setTimezone(DateTimeZone $timezone) : DateTime { }
  public function sub(DateInterval $interval) : DateTime { }
  public function __wakeup() { }
  public static function __set_state(array $array) : DateTime { }
}
class DateTimeImmutable implements DateTimeInterface {
  public function __construct(string $time = "now", ?DateTimeZone $timezone = null) { }
  public function add(DateInterval $interval) : DateTimeImmutable { }
  public static function createFromFormat(string $format,
                                          string $time,
                                          ?DateTimeZone $timezone = null) : DateTimeImmutable { }
  public static function getLastErrors() : array { }
  public function modify(string $modify) : DateTimeImmutable { }
  public static function __set_state(array $array) : DateTimeImmutable { }
  public function setDate(int $year, int $month, int $day) : DateTimeImmutable { }
  public function setISODate(int $year, int $week, int $day = 1) : DateTimeImmutable { }
  public function setTime(int $hour, int $minute, int $second = 0) : DateTimeImmutable { }
  public function setTimestamp(int $unixtimestamp) : DateTimeImmutable { }
  public function setTimezone(DateTimeZone $timezone) : DateTimeImmutable { }
  public function sub(DateInterval $interval) : DateTimeImmutable { }
  public function diff(DateTimeInterface $datetime2, bool $absolute = false) : DateInterval { }
  public function format(string $format) : string { }
  public function getOffset() : int { }
  public function getTimestamp() :int { }
  public function getTimezone() : DateTimeZone { }
  public function __wakeup() { }
}
class DateTimeZone {
  const AFRICA = 0;
  const AMERICA = 0;
  const ANTARCTICA = 0;
  const ARCTIC = 0;
  const ASIA = 0;
  const ATLANTIC = 0;
  const AUSTRALIA = 0;
  const EUROPE = 0;
  const INDIAN = 0;
  const PACIFIC = 0;
  const UTC = 0;
  const ALL = 0;
  const ALL_WITH_BC = 0;
  const PER_COUNTRY = 0;
  public function __construct($timezone) { }
  public function getLocation() : array<string, mixed> { }
  public function getName() : string { }
  public function getOffset(DateTime $datetime) : int { }
  public function getTransitions(int $timestamp_begin = PHP_INT_MIN,
                                 int $timestamp_end = PHP_INT_MAX) : array<array<string, mixed>> { }
  public static function listAbbreviations() : mixed { }
  public static function listIdentifiers(int $what = DateTimeZone::ALL, ?string $country = null) : mixed { }
}
class DateInterval {
  public function __construct($interval_spec) { }
  public function __get($member) { }
  public function __set($member, $value) { }
  static public function createFromDateString($time) { }
  public function format($format) { }
}
class DatePeriod implements Traversable {
  const int EXCLUDE_START_DATE = 1;

  public function __construct (mixed $a, mixed $b = null, mixed $c=null, int $options=0) { }
}

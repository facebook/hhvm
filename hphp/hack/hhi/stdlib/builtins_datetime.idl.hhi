<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function checkdate($month, $day, $year) { }
function date_add($datetime, $interval) { }
function date_create_from_format($format, $time, $timezone = null_object) { }
function date_create($time = null, $timezone = null_object) { }
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
function date($format, $timestamp = null) { }
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
function time() { }
function timezone_abbreviations_list() { }
function timezone_identifiers_list() { }
function timezone_location_get($timezone) { }
function timezone_name_from_abbr($abbr, $gmtoffset = -1, $isdst = true) { }
function timezone_name_get($object) { }
function timezone_offset_get($object, $dt) { }
function timezone_open($timezone) { }
function timezone_transitions_get($object) { }
function timezone_version_get() { }
class DateTime {
  const ATOM = 0;
  const COOKIE = 0;
  const ISO8601 = 0;
  const RFC822 = 0;
  const RFC850 = 0;
  const RFC1036 = 0;
  const RFC1123 = 0;
  const RFC2822 = 0;
  const RFC3339 = 0;
  const RSS = 0;
  const W3C = 0;
  public function add($interval) { }
  public function __construct($time = "now", $timezone = null_object) { }
  static public function createFromFormat($format, $time, $timezone = null_object) { }
  public function diff($datetime2, $absolute = false) { }
  public function format($format) { }
  static public function getLastErrors() { }
  public function getOffset() { }
  public function getTimestamp() { }
  public function getTimezone() { }
  public function modify($modify) { }
  public function setDate($year, $month, $day) { }
  public function setISODate($year, $week, $day = 1) { }
  public function setTime($hour, $minute, $second = 0) { }
  public function setTimestamp($unixtimestamp) { }
  public function setTimezone($timezone) { }
  public function sub($interval) { }
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
  public function getLocation() { }
  public function getName() { }
  public function getOffset($datetime) { }
  public function getTransitions() { }
  static public function listAbbreviations() { }
  static public function listIdentifiers() { }
}
class DateInterval {
  public function __construct($interval_spec) { }
  public function __get($member) { }
  public function __set($member, $value) { }
  static public function createFromDateString($time) { }
  public function format($format) { }
}

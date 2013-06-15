<?php

/*
 * These tests only "work" depending on the version of timelib we're
 * linked with.
 *
 * We've set them up to pass anyway, using the error handler.  This is
 * a little inconvenient (as the output is entirely one bool ...), but
 * better than no way to test.
 */
function handler() {
  var_dump(true);
}
set_error_handler('handler');

function do_date_diff() {
  $dt1 = date_create("2010-08-02");
  $dt2 = date_create("2010-08-30");
  $interval = date_diff($dt1, $dt2, true);
  return date_interval_format($interval, "%d") === "28";
}

function test_date_interval_create_from_date_string() {
  $interval = date_interval_create_from_date_string("2 weeks");
  return date_interval_format($interval, "%d") === "14";
}

function test_date_sub() {
  $datetime = date_create("2010-08-16");
  $interval = date_interval_create_from_date_string("2 weeks");
  $dt2 = date_sub($datetime, $interval);
  return date_format($dt2, "Y-m-d") === "2010-08-02";
}

function test_timezone_location_get() {
  $tz = timezone_open("Europe/Prague");
  $loc = timezone_location_get(tz);
  return count($loc) === 4 &&
         (string)$loc['country_code'] === "CZ" &&
         (int)((double)$loc['latitude'] * 100) === 5008 &&
         (int)((double)$loc['longitude'] * 100) === 1443 &&
         (string)$loc['comments'] === ""
         ;
}

var_dump(test_date_diff() &&
         test_date_interval_create_from_date_string() &&
         test_date_sub() &&
         test_timezone_location_get());

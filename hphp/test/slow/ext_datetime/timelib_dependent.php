<?hh

function test_date_diff() :mixed{
  $dt1 = date_create("2010-08-02");
  $dt2 = date_create("2010-08-30");
  $interval = date_diff($dt1, $dt2, true);
  return date_interval_format($interval, "%d");
}

function test_date_interval_create_from_date_string() :mixed{
  $interval = date_interval_create_from_date_string("2 weeks");
  return date_interval_format($interval, "%d");
}

function test_date_sub() :mixed{
  $datetime = date_create("2010-08-16");
  $interval = date_interval_create_from_date_string("2 weeks");
  $dt2 = date_sub($datetime, $interval);
  return date_format($dt2, "Y-m-d");
}

function test_timezone_location_get() :mixed{
  $tz = timezone_open("Europe/Prague");
  $loc = timezone_location_get($tz);
  return $loc;
}


<<__EntryPoint>>
function main_timelib_dependent() :mixed{
var_dump(test_date_diff());
var_dump(test_date_interval_create_from_date_string());
var_dump(test_date_sub());
var_dump(test_timezone_location_get());
}

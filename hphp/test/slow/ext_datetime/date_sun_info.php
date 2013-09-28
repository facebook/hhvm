<?php

date_default_timezone_set("America/Los_Angeles");

var_dump(date_sun_info(strtotime("2006-12-12"), 31.7667, 35.2333));

/*
 * calculate the sunrise time for Lisbon, Portugal
 * Latitude: 38.4 North
 * Longitude: 9 West
 * Zenith ~= 90
 * offset: +1 GMT
 */
var_dump(
  date_sunrise(strtotime("2004-12-20"), SUNFUNCS_RET_STRING,
               38.4, -9, 90, 1)
);

/*
 * calculate the sunset time for Lisbon, Portugal
 * Latitude: 38.4 North
 * Longitude: 9 West
 * Zenith ~= 90
 * offset: +1 GMT
 */
var_dump(
  date_sunset(strtotime("2004-12-20"), SUNFUNCS_RET_STRING,
              38.4, -9, 90, 1)
);

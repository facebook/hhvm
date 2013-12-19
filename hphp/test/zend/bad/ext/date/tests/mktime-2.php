<?php
$timezones = array(
    'UTC',
    'Europe/London'
);

foreach($timezones as $timezone)
{
    date_default_timezone_set($timezone);

    /* status of daylight saving time unknown */
    var_dump(mktime(0, 0, 0, 1, 1, 2002));
    /* status of daylight saving time unknown */
    var_dump(mktime(0, 0, 0, 1, 1, 2002, -1));
    /* daylight saving time is not in effect */
    var_dump(mktime(0, 0, 0, 1, 1, 2002, 0));
    /* daylight saving time is in effect */
    var_dump(mktime(0, 0, 0, 1, 1, 2002, 1));

    /* status of daylight saving time unknown */
    var_dump(mktime(0, 0, 0, 7, 1, 2002));
    /* status of daylight saving time unknown */
    var_dump(mktime(0, 0, 0, 7, 1, 2002, -1));
    /* daylight saving time is not in effect */
    var_dump(mktime(0, 0, 0, 7, 1, 2002, 0));
    /* daylight saving time is in effect */
    var_dump(mktime(0, 0, 0, 7, 1, 2002, 1));
}
?>
<?php

function formatted_time($datetime_str = 'now', $timestamp_format = NULL, $timezone = NULL)
{
    $tz   = new DateTimeZone($timezone ? $timezone : date_default_timezone_get());
    $time = new DateTime($datetime_str, $tz);
    if ($time->getTimeZone()->getName() !== $tz->getName())
    {
        $time->setTimeZone($tz);
    }
    return $time->format($timestamp_format);
}

$datetime_str= '@1301574225';

$timestamp_format='Y-m-d H:i:s e';


$timezone='Antarctica/South_Pole';


$timestamp = formatted_time($datetime_str, $timestamp_format, $timezone);


var_dump($timestamp);

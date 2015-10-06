<?php

function dump_transitions($tz) {
    $timestamp_begin = 1301574225;
    $timestamp_end = 1401574225;

    $tz = new DateTimeZone($tz);

    var_dump($tz->getTransitions($timestamp_begin, $timestamp_end));
}

dump_transitions('UTC');
dump_transitions('Pacific/Auckland');

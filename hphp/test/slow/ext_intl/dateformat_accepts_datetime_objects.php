<?php

$formatter = new IntlDateFormatter('en', IntlDateFormatter::FULL,
                                   IntlDateFormatter::FULL);
$now = new DateTime();
var_dump($formatter->format($now) ===
         $formatter->format($now->getTimestamp()));

// ensure nothing blows up when attempting to format objects of the wrong type
$formatter->format(new stdClass());

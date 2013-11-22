<?php

$start = new DateTime('2012-07-01');
$ival = new DateInterval('P7D');
$end = new DateTime('2012-07-31');
$recurrences = 4;

// All of these periods are equivalent.
$period = new DatePeriod($start, $ival, $recurrences);

foreach ($period as $key => $date) {
  echo $key . " " . $date->format('Y-m-d') . "\n";
}

$period = new DatePeriod($start, $ival, $end, DatePeriod::EXCLUDE_START_DATE);

foreach ($period as $key => $date) {
  echo $key . " " . $date->format('Y-m-d') . "\n";
}

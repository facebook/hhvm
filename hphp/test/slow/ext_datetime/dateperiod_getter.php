<?php

$period = new DatePeriod(
  new DateTime("2015-01-01"),
  new DateInterval("PT6H"),
  new DateTime("2015-01-02")
);

echo 'Starting Date = ', $period->getStartDate()->format('Y-m-d'), PHP_EOL;
echo 'Ending Date = ', $period->getEndDate()->format('Y-m-d'), PHP_EOL;
echo 'Interval = ', $period->getDateInterval()->format('%H hours'), PHP_EOL;

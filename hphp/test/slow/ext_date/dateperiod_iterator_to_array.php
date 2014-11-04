<?php

date_default_timezone_set("Europe/Berlin");

$period = new DatePeriod(
    new DateTime('2010-05-01'),
    new DateInterval('P1D'),
    new DateTime('2010-05-10')
);
foreach (iterator_to_array($period) as $date) {
    echo $date->format("l Y-m-d H:i:s\n");
    echo PHP_EOL;
}

<?php

$dateperiod = new \DatePeriod('R4/2017-05-01T00:00:00Z/P1D');

foreach ($dateperiod as $date) {
    print $date->format(DateTime::ATOM)."\n";
}

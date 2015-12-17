<?php

$dt1 = new DateTime();
$ti = DateInterval::createFromDateString('P1D');
$dt2 = $dt1->add($ti);
var_dump($dt2);

<?php
date_default_timezone_set('America/Los_Angeles');
$a = new \DateTime('2013-01-28');
$b = clone $a;
$interval = new \DateInterval('P1D');
$b->sub($interval);
$c = $a->diff($b)->format('%y/%m/%d %H:%I:%S');
echo $c;

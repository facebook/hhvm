<?php

$d1=new DateTimeImmutable('03/08/2015 1:59am GMT-05:00');
echo $d1->format(DateTime::ISO8601)."\n";

$d2 = $d1->modify('+0 hours');
echo $d2->format(DateTime::ISO8601)."\n";

echo "--\n";

$d1=new DateTime('03/08/2015 1:59am GMT-05:00');
echo $d1->format(DateTime::ISO8601)."\n";

$d2 = clone $d1;
echo $d2->format(DateTime::ISO8601)."\n";

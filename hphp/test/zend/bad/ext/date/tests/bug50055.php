<?php
$now = '2010-03-07 13:21:38 UTC';
//positive DateInterval
$da1 = date_create( $now );
$ds1 = date_create( $now );
$i = DateInterval::createFromDateString('third Tuesday of next month');
echo $da1->format( DateTime::ISO8601 ), "\n";
echo date_add($da1, $i)->format( DateTime::ISO8601 ), "\n";
date_sub($ds1, $i);

//negative DateInterval
$da2 = date_create( $now );
$ds2 = date_create( $now );
$i2 = DateInterval::createFromDateString('third Tuesday of last month');
echo $da2->format( DateTime::ISO8601 ), "\n";
echo date_add($da2, $i2)->format( DateTime::ISO8601 ), "\n";//works
date_sub($ds2, $i);
?>

<?php
$begin = new DateTime( '2012-08-30' );
$end = new DateTime( '2012-08-31' );

$interval = new DateInterval('P1D');
$daterange = new DatePeriod($begin, $interval ,$end);

foreach($daterange as $date){
    echo $date->format("Ymd") . PHP_EOL;
}

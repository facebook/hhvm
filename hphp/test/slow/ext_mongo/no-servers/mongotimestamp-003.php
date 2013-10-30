<?php
$ts = new MongoTimestamp('60', '30');
var_dump($ts->sec);
var_dump($ts->inc);

$ts = new MongoTimestamp(60.123, 3e1);
var_dump($ts->sec);
var_dump($ts->inc);
?>

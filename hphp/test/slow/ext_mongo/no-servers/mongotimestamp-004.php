<?php
$ts1 = new MongoTimestamp(60, 30);
$ts2 = new MongoTimestamp(60, 30);
var_dump($ts1 == $ts2);
var_dump($ts1 === $ts2);
?>

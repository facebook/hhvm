<?php
$date = new MongoDate();
var_dump($date->usec === ($date->usec / 1000) * 1000);
?>

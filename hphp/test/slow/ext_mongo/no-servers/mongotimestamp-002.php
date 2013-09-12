<?php
$ts = new MongoTimestamp();
var_dump(time() - $ts->sec <= 1);
?>

<?php
$tz = date_timestamp_get(new DateTime()); 
var_dump(is_int($tz));
echo "\n\n";
$tz = date_timestamp_get(time());
?>
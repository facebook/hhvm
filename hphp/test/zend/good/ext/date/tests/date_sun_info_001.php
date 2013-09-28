<?php
date_default_timezone_set('UTC');
$sun_info = date_sun_info(strtotime("2006-12-12"), 31.7667, 35.2333);
var_dump($sun_info);
echo "Done\n";
?>
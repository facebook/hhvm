<?php
$before = xdebug_time_index();
for ($i = 0; $i < 250000; $i++) { }
$after = xdebug_time_index();
var_dump($before);
var_dump($after);
var_dump($after - $before >= 0);

<?php
var_dump($a = memory_get_peak_usage());
var_dump(memory_get_peak_usage(true));
var_dump(memory_get_peak_usage(false));
$array = range(1,1024*1024);
var_dump(memory_get_peak_usage() > $a);
?>

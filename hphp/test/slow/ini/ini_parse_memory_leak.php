<?php

error_reporting(0);
$total_run = 10000;
function leak_test()
{
    global $total_run;
    $count = $total_run;
    $before = memory_get_usage(true);
    while ($count) {
        $count--;
        parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }
}

$a = memory_get_usage(true);
leak_test();
$b = memory_get_usage(true);

$diff1 = $b - $a;

if ($diff1 < 8192) {
    echo "OK";
} else {
    echo "ini parse memory leak, total:".$diff1;
}

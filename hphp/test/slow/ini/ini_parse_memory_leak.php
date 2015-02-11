<?php

error_reporting(0);
$total_run = 100000;

function empty_func()
{

}

function leak_test()
{
    global $total_run;
    $count = $total_run;
    while ($count) {
        $count--;
        parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }
}

$a = memory_get_usage(true);
empty_func();
$b = memory_get_usage(true);
leak_test();
$c = memory_get_usage(true);

$diff1 = $b - $a;
$diff2 = $c - $b;

if ($diff2 - $diff1 < 81920) {
    echo "OK";
} else {
    echo "ini parse memory leak, total:".$diff1;
}

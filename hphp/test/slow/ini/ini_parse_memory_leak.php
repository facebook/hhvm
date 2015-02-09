<?php

error_reporting();
function test()
{
    $count = 10000;
    $before = memory_get_usage(true);
    while ($count) {
        $count--;
        parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }
}

$a = memory_get_usage(true);
test();
$b = memory_get_usage(true);
test();
$c = memory_get_usage(true);

$diff1 = $b - $a;
$diff2 = $c - $b;

if ($diff2 * 2 <= $diff1) {
    echo "OK";
} else {
    echo "ini parse memory leak";
}

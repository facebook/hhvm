<?php

error_reporting(0);
function empty_func()
{

}

function leak_test()
{
    $a = memory_get_usage(true);
    for ($i = 0; $i < 100000; $i++) {
        parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }
    $b = memory_get_usage(true);
    $first_leak = $b - $c;

    for ($i = 0; $i < 200000; $i++) {
        parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }

    $c = memory_get_usage(true);
    $second_leak = $c - $b;

    $count = $second_leak / $first_leak;
    if ($count <= 1) {
        echo "OK";
    } else {
        echo "leak first:".$first_leak.", leak second:".$second_leak.", times:".$count."\n";
    }
}

leak_test();

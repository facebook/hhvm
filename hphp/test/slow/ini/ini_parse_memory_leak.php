<?php

error_reporting(0);

function leak_test()
{
    $a = memory_get_usage(true);
    parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    $b = memory_get_usage(true);
    $diff = $b - $a;

    $c = memory_get_usage(true);
    for ($i = 0; $i < 10000; $i++) {
        parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }
    $d = memory_get_usage(true);

    $total=$d - $c;
    $count = 0;
    if ($diff > 0) {
        $count = $total/$diff;
    }

    if ($count <= 3) {
        echo "OK\n";
    } else {
        echo "diff:".$diff.",total:".$total."\n"; 
        echo "count:".$count."\n";
    }
}

leak_test();

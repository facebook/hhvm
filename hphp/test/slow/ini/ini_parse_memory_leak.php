<?php

//error_reporting(0);

function leak_test($output)
{
    $a = memory_get_usage(true);
    for ($i = 0; $i < 1000; $i++) {
        $data=parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }
    $b = memory_get_usage(true);
    $first_leak = $b - $a;

    for ($i = 0; $i < 2000; $i++) {
        $data=parse_ini_file('./ini_parse_memory_leak_test.ini', true);
    }

    $c = memory_get_usage(true);
    $second_leak = $c - $a;

    if ($output === false) {
        return;
    }

    $count = 0;
    if ($first_leak > 0) {
        $count = $second_leak / $first_leak;
    }

    if ($count <= 1.25) {
        echo "OK";
    } else {
        echo "leak first:".$first_leak.", leak second:".$second_leak.", times:".$count."\n";
    }
}

leak_test(false);
leak_test(false);
leak_test(true);

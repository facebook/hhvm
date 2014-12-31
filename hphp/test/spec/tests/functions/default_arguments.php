<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// default argument values; must be constants (or intrinsic function calls like array)

///*
function f1($p1 = 10, $p2 = 1.23, $p3 = TRUE, $p4 = NULL, $p5 = "abc", $p6 = [1,2,3,array()])
{
    $argList = func_get_args();
    echo "f1: # arguments passed is ".count($argList)."\n";

    foreach ($argList as $k => $e)
    {
        echo "\targ[$k] = >$e<\n";
    }
    echo "\$p1: $p1, \$p2: $p2, \$p3: $p3, \$p4: $p4, \$p5: $p5, \$p6: $p6\n";
}

f1();
f1(20);
f1(10, TRUE);
f1(NULL, 12, 1.234);
f1(FALSE, 12e2, [99,-99], "abc");
f1(9, 8, 7, 6, 5);
f1(10, 20, 30, 40, 50, 60);
f1(1, 2, 3, 4, 5, 6, 7);
//*/
///*
// 2 default followed by one non-default; unusual, but permitted

function f2($p1 = 100, $p2 = 1.23, $p3)
{
    $argList = func_get_args();
    echo "f2: # arguments passed is ".count($argList)."\n";

    foreach ($argList as $k => $e)
    {
        echo "\targ[$k] = >$e<\n";
    }
    echo "\$p1: ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2: ".($p2 == NULL ? "NULL" : $p2).
        ", \$p3: ".($p3 == NULL ? "NULL" : $p3)."\n";
}

f2();
f2(10);
f2(10, 20);
f2(10, 20, 30);
//*/
///*
// 1 default followed by one non-default followed by 1 default; unusual, but permitted

function f3($p1 = 100, $p2, $p3 = "abc")
{
    $argList = func_get_args();
    echo "f3: # arguments passed is ".count($argList)."\n";

    foreach ($argList as $k => $e)
    {
        echo "\targ[$k] = >$e<\n";
    }
    echo "\$p1: ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2: ".($p2 == NULL ? "NULL" : $p2).
        ", \$p3: ".($p3 == NULL ? "NULL" : $p3)."\n";
}

f3();
f3(10);
f3(10, 20);
f3(10, 20, 30);
//*/
///*
// 1 non-default followed by two default; unusual, but permitted

function f4($p1, $p2 = 1.23, $p3 = "abc")
{
    $argList = func_get_args();
    echo "f4: # arguments passed is ".count($argList)."\n";

    foreach ($argList as $k => $e)
    {
        echo "\targ[$k] = >$e<\n";
    }
    echo "\$p1: ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2: ".($p2 == NULL ? "NULL" : $p2).
        ", \$p3: ".($p3 == NULL ? "NULL" : $p3)."\n";
}

f4();
f4(10);
f4(10, 20);
f4(10, 20, 30);
//*/

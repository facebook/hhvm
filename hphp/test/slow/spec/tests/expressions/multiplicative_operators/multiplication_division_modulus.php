<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$oper = array(0, -10, 100, -3.4e10, INF, -INF, NAN, TRUE, FALSE, NULL,
    "123", "2e+5", "", "abc", PHP_INT_MAX );
///*
foreach ($oper as $e1)
{
    foreach ($oper as $e2)
    {
        echo ">$e1< * >$e2<, result: "; var_dump($e1 * $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $e1)
{
    foreach ($oper as $e2)
    {
        if (($e2) == 0) continue;   // skip divide-by-zeros

        echo ">$e1< / >$e2<, result: "; var_dump($e1 / $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $e1)
{
    foreach ($oper as $e2)
    {
        if (((int)$e2) == 0) continue;  // skip divide-by-zeros

        echo "      >$e1< % >$e2<, result: "; var_dump($e1 % $e2);
        echo "(int) >".((int)$e1)."< % >(int) >".((int)$e2)."<, result: "; var_dump(((int)$e1) % ((int)$e2));
    }
    echo "-------------------------------------\n";
}
//*/

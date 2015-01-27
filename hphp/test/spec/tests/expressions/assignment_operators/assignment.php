<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$oper = array(0, -10, 100, -3.4e10, INF, -INF, NAN, TRUE, FALSE, NULL,
    "123", "2e+5", "", "abc", PHP_INT_MAX );
///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< *= >$e2<, result: "; var_dump($e1 *= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        if (($e2) == 0) continue;   // skip divide-by-zeros

        $e1 = $t;
        echo ">$e1< /= >$e2<, result: "; var_dump($e1 /= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        if (((int)$e2) == 0) continue;  // skip divide-by-zeros

        $e1 = $t;
        echo ">$e1< %= >$e2<, result: "; var_dump($e1 %= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< += >$e2<, result: "; var_dump($e1 += $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< -= >$e2<, result: "; var_dump($e1 -= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< .= >$e2<, result: "; var_dump($e1 .= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< <<= >$e2<, result: "; var_dump($e1 <<= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< >>= >$e2<, result: "; var_dump($e1 >>= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< &= >$e2<, result: "; var_dump($e1 &= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< ^= >$e2<, result: "; var_dump($e1 ^= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
foreach ($oper as $t)
{
    foreach ($oper as $e2)
    {
        $e1 = $t;
        echo ">$e1< |= >$e2<, result: "; var_dump($e1 |= $e2);
    }
    echo "-------------------------------------\n";
}
//*/

///*
var_dump($v = 10);
var_dump($v += 20);
var_dump($v -= 5);
var_dump($v .= 123.45);
$a = [100, 200, 300];
$i = 1;
var_dump($a[$i++] += 50);
var_dump($i);
//*/

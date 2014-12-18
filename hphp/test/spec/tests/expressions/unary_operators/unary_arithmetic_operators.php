<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function DoIt($a)
{
    echo "--- start DoIt -------------------------\n\n";
    echo "     original: "; var_dump($a);
    $b = +$a;
//  echo "after unary +: "; var_dump($b);
    $c = -$a;
//  echo "after unary -: "; var_dump($c);
    $d = !$a;
//  echo "after unary !: "; var_dump($d);
    $e = ($a == 0);
//  echo "after $a == 0: "; var_dump($e);
/*
    $f = ~$a;
    echo "after unary ~: "; var_dump($f);
    printf(" before Hex: %08X\n", $a);
    printf(" after  Hex: %08X\n", $f);

    echo " before (int): ".(int)$a;
    printf("; before (int) Hex: %08X\n", $a);
*/
    echo "\n--- end DoIt -------------------------\n\n";
}

///*
// arithmetic operands

DoIt(0);
DoIt(5);
DoIt(-10);
DoIt(PHP_INT_MAX);
DoIt(-PHP_INT_MAX - 1);
DoIt(0.0);
DoIt(0.0000001e-100);
DoIt(12.7345);
DoIt(-9.34E26);
DoIt(PHP_INT_MAX + 10);
DoIt(1234567E50);
DoIt(1234567E100);
DoIt(INF);
DoIt(-INF);
DoIt(NAN);
DoIt(-NAN);
//*/

///*
// NULL operand

DoIt(NULL);         // ~ not supported, so disable cod eblock in DoIt when testing
//*/

//*
// Boolean operands

DoIt(TRUE);         // ~ not supported, so disable code block in DoIt when testing
DoIt(FALSE);        // ~ not supported, so disable code block in DoIt when testing
//*/

///*
// string operands

DoIt("0");
DoIt("-43");
DoIt("123");
DoIt("0.0");
DoIt("-25.5e-10");
DoIt("");
DoIt("ABC");
//*/

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
// Numeric strings with all kinds of operands, swapping them over to make
// LHS/RHS order is irrelevent.

$oper1 = array("10", "-5.1");
$oper2 = array(0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", [], [10,2.3]);

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} >        {$e2}  result: "; var_dump($e1 > $e2);
        echo "{$e2} <=       {$e1}  result: "; var_dump($e2 <= $e1);
        echo "---\n";
        echo "{$e1} >=       {$e2}  result: "; var_dump($e1 >= $e2);
        echo "{$e2} <        {$e1}  result: "; var_dump($e2 < $e1);
        echo "---\n";
        echo "{$e1} <        {$e2}  result: "; var_dump($e1 < $e2);
        echo "{$e2} >=       {$e1}  result: "; var_dump($e2 >= $e1);
        echo "---\n";
        echo "{$e1} <=       {$e2}  result: "; var_dump($e1 <= $e2);
        echo "{$e2} >        {$e1}  result: "; var_dump($e2 > $e1);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}

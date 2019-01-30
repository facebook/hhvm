<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
// checkout the type of the result

$a = 10 == 20;
var_dump($a);
$a = 10 != 20;
var_dump($a);
$a = 10 <> "xxx";
var_dump($a);
$a = "zz" === "xx";
var_dump($a);
$a = "zz" !== "zz";
var_dump($a);
echo "\n";
//*/

///*
// NULL operand with all kinds of operands

$oper1 = array(NULL);
$oper2 = array(0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", [], [10,2.3]);

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
        echo "{$e1} <>   {$e2}  result: "; var_dump($e1 <> $e2);
        echo "{$e1} ===  {$e2}  result: "; var_dump($e1 === $e2);
        echo "{$e1} !==  {$e2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Two non-numeric strings

$oper1 = array("", "a", "aa");
$oper2 = array("", "aa", "A", "AB");

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
        echo "{$e1} <>   {$e2}  result: "; var_dump($e1 <> $e2);
        echo "{$e1} ===  {$e2}  result: "; var_dump($e1 === $e2);
        echo "{$e1} !==  {$e2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Boolean operand with all kinds of operands

$oper1 = array(TRUE, FALSE);
$oper2 = array(0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", [], [10,2.3]);

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
        echo "{$e1} <>   {$e2}  result: "; var_dump($e1 <> $e2);
        echo "{$e1} ===  {$e2}  result: "; var_dump($e1 === $e2);
        echo "{$e1} !==  {$e2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Numeric strings with all kinds of operands

$oper1 = array("10", "-5.1");
$oper2 = array(0, 10, -3.4, TRUE, FALSE, NULL, "", "123", "abc", [], [10,2.3]);

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
        echo "{$e1} <>   {$e2}  result: "; var_dump($e1 <> $e2);
        echo "{$e1} ===  {$e2}  result: "; var_dump($e1 === $e2);
        echo "{$e1} !==  {$e2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Two array types

$oper1 = array([10,20], ["red"=>0,"green"=>0]);
$oper2 = array([10,20.0], [10,20,30], ["red"=>0,"green"=>0], ["green"=>0,"red"=>0]);

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
        echo "{$e1} <>   {$e2}  result: "; var_dump($e1 <> $e2);
        echo "{$e1} ===  {$e2}  result: "; var_dump($e1 === $e2);
        echo "{$e1} !==  {$e2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

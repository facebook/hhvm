<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

///*
// checkout the type of the result

$a = 10 == 20;
var_dump($a);
$a = 10 != 20;
var_dump($a);
var_dump($a);
$a = "zz" === "xx";
var_dump($a);
$a = "zz" !== "zz";
var_dump($a);
echo "\n";
//*/

///*
// NULL operand with all kinds of operands

$oper1 = varray[NULL];
$oper2 = varray[0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", varray[], varray[10,2.3]];

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
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

$oper1 = varray["", "a", "aa"];
$oper2 = varray["", "aa", "A", "AB"];

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
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

$oper1 = varray[TRUE, FALSE];
$oper2 = varray[0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", varray[], varray[10,2.3]];

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
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

$oper1 = varray["10", "-5.1"];
$oper2 = varray[0, 10, -3.4, TRUE, FALSE, NULL, "", "123", "abc", varray[], varray[10,2.3]];

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
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

$oper1 = varray[varray[10,20], darray["red"=>0,"green"=>0]];
$oper2 = varray[varray[10,20.0], varray[10,20,30], darray["red"=>0,"green"=>0], darray["green"=>0,"red"=>0]];

foreach ($oper1 as $e1)
{
    foreach ($oper2 as $e2)
    {
        echo "{$e1} ==   {$e2}  result: "; var_dump($e1 == $e2);
        echo "{$e1} !=   {$e2}  result: "; var_dump($e1 != $e2);
        echo "{$e1} ===  {$e2}  result: "; var_dump($e1 === $e2);
        echo "{$e1} !==  {$e2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/
}

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

$oper1 = vec[NULL];
$oper2 = vec[0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", vec[], vec[10,2.3]];

foreach ($oper1 as $e1)
{
    $t1 = HH\is_any_array($e1) ? 'Array' : (string)$e1;
    foreach ($oper2 as $e2)
    {
        $t2 = HH\is_any_array($e2) ? 'Array' : (string)$e2;
        echo "{$t1} ==   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\eq($e1, $e2));
        echo "{$t1} !=   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\neq($e1, $e2));
        echo "{$t1} ===  {$t2}  result: "; var_dump($e1 === $e2);
        echo "{$t1} !==  {$t2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Two non-numeric strings

$oper1 = vec["", "a", "aa"];
$oper2 = vec["", "aa", "A", "AB"];

foreach ($oper1 as $e1)
{
    $t1 = HH\is_any_array($e1) ? 'Array' : $e1;
    foreach ($oper2 as $e2)
    {
        $t2 = HH\is_any_array($e2) ? 'Array' : $e2;
        echo "{$t1} ==   {$t2}  result: "; var_dump($e1 == $e2);
        echo "{$t1} !=   {$t2}  result: "; var_dump($e1 != $e2);
        echo "{$t1} ===  {$t2}  result: "; var_dump($e1 === $e2);
        echo "{$t1} !==  {$t2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Boolean operand with all kinds of operands

$oper1 = vec[TRUE, FALSE];
$oper2 = vec[0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", vec[], vec[10,2.3]];

foreach ($oper1 as $e1)
{
    $t1 = HH\is_any_array($e1) ? 'Array' : (string)$e1;
    foreach ($oper2 as $e2)
    {
        $t2 = HH\is_any_array($e2) ? 'Array' : (string)$e2;
        echo "{$t1} ==   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\eq($e1, $e2));
        echo "{$t1} !=   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\neq($e1, $e2));
        echo "{$t1} ===  {$t2}  result: "; var_dump($e1 === $e2);
        echo "{$t1} !==  {$t2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Numeric strings with all kinds of operands

$oper1 = vec["10", "-5.1"];
$oper2 = vec[0, 10, -3.4, TRUE, FALSE, NULL, "", "123", "abc", vec[], vec[10,2.3]];

foreach ($oper1 as $e1)
{
    $t1 = HH\is_any_array($e1) ? 'Array' : (string)$e1;
    foreach ($oper2 as $e2)
    {
        $t2 = HH\is_any_array($e2) ? 'Array' : (string)$e2;
        echo "{$t1} ==   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\eq($e1, $e2));
        echo "{$t1} !=   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\neq($e1, $e2));
        echo "{$t1} ===  {$t2}  result: "; var_dump($e1 === $e2);
        echo "{$t1} !==  {$t2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/

///*
// Two array types

$oper1 = vec[vec[10,20], dict["red"=>0,"green"=>0]];
$oper2 = vec[vec[10,20.0], vec[10,20,30], dict["red"=>0,"green"=>0], dict["green"=>0,"red"=>0]];

foreach ($oper1 as $e1)
{
    $t1 = HH\is_any_array($e1) ? 'Array' : (string)$e1;
    foreach ($oper2 as $e2)
    {
        $t2 = HH\is_any_array($e2) ? 'Array' : (string)$e2;
        echo "{$t1} ==   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\eq($e1, $e2));
        echo "{$t1} !=   {$t2}  result: "; var_dump(HH\Lib\Legacy_FIXME\neq($e1, $e2));
        echo "{$t1} ===  {$t2}  result: "; var_dump($e1 === $e2);
        echo "{$t1} !==  {$t2}  result: "; var_dump($e1 !== $e2);
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n";
//*/
}

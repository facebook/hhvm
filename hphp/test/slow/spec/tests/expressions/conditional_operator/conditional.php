<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function f($a) :mixed{ echo "inside f($a)\n"; return 0;}
<<__EntryPoint>> function main(): void {
error_reporting(-1);

// check for even integer values by inspecting the low-order bit

for ($i = -5; $i <= 5; ++$i)
    echo "$i is ".(($i & 1) === 1 ? "odd\n" : "even\n");

// some simple examples

$a = 10 ? 100 : "Hello";
var_dump($a);
$a = 0 ? 100 : "Hello";
var_dump($a);

// omit 2nd operand

$a = 10 ? : "Hello";
var_dump($a);
$a = 0 ? : "Hello";
var_dump($a);

// put a side effect in the 1st operand

$i = 5;
$a = $i++ ? : "red";
var_dump($a);
$i = 5;
$a = ++$i ? : "red";
var_dump($a);

$i = PHP_INT_MAX;
$a = $i++ ? : "red";
var_dump($a);
$i = PHP_INT_MAX;
$a = ++$i ? : "red";
var_dump($a);

// sequence point

$i = 5;
$i++ ? f($i) : f(++$i);
$i = 0;
$i++ ? f($i) : f(++$i);

// Test all kinds of scalar values to see which are ints or can be implicitly converted

$scalarValueList = vec[10, -100, 0, 1.234, 0.0, TRUE, FALSE, NULL, "123", 'xx', ""];
foreach ($scalarValueList as $v)
{
    echo "\$v = ".(string)$v.", ";
    $a = $v ? 100 : "Hello";
    var_dump($a);
}

// check associativity -- NOT the same as C/C++

$a = TRUE ? -1 : TRUE ? 10 : 20;
var_dump($a);
$a = (TRUE ? -1 : TRUE) ? 10 : 20;
var_dump($a);
$a = TRUE ? -1 : (TRUE ? 10 : 20);
var_dump($a);
}

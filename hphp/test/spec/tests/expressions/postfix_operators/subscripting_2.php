<?php

echo "====== array without index; simple assignment =========\n";

$a = array(3 => 33, -1 => -11);
var_dump($a[] = 991);       // creates $a[4]
var_dump($a);
echo "------\n";

$a = array(-30 => 33, -10 => -11);
var_dump($a[] = 991);       // creates $a[0]
var_dump($a);
echo "------\n";

$a = array(0 => 33, -10 => 11);
var_dump($a[] = 991);       // creates $a[1]
var_dump($a);
echo "------\n";

$a = array('a' => 33, 'x' => -11);
var_dump($a[] = 991);       // creates $a[0]
var_dump($a);

echo "====== array without index; compound assignment =========\n";

$a = array('a' => 33, 'x' => -11);
var_dump($a[] += 991);      // creates $a[0]
var_dump($a);
echo "------\n";

$a = array('a' => 33, 'x' => -11);
var_dump($a[] -= 991);      // creates $a[0]
var_dump($a);
echo "------\n";

$a = array('a' => 33, 'x' => -11);
var_dump($a[] *= 991);      // creates $a[0]
var_dump($a);

echo "====== array without index; ++/-- =========\n";

$a = array(3 => 33, -1 => -11);
//$a = array('x' => 33, 'y' => -11);
//$a = array();
var_dump($a);
echo "------\n";

//var_dump($a[]);

var_dump($a[]++);
var_dump($a);
echo "------\n";

var_dump(++$a[]);
var_dump($a);
echo "------\n";

var_dump(--$a[]);
var_dump($a);

echo "====== object; set up =========\n";

class C10 implements ArrayAccess
{
    function offsetExists($offset)
    {
        echo "\nInside " . __METHOD__ . "\n"; var_dump($offset);
    }
    function offsetGet($offset)
    {
        echo "\nInside " . __METHOD__ . "\n"; var_dump($offset); return 100;
    }
    function offsetSet($offset, $value)
    {
        echo "\nInside " . __METHOD__ . "\n"; var_dump($offset); var_dump($value);
    }
    function offsetUnset($offset)
    {
        echo "\nInside " . __METHOD__ . "\n"; var_dump($offset);
    }
}

$c10 = new C10;

echo "====== object with index; as non-lvalue =========\n";

var_dump($c10[1]);
echo "------\n";

var_dump($c10[1000]);
echo "------\n";

var_dump($c10[-123]);
echo "------\n";

var_dump($c10['abc']);

echo "====== object with index; simple assignment =========\n";

var_dump($c10[1] = 34);
echo "------\n";

var_dump($c10[1000] = 34);
echo "------\n";

var_dump($c10[-123] = 34);
echo "------\n";

var_dump($c10['abc'] = 34);

echo "====== object with index; compound assignment =========\n";

var_dump($c10[1000] += 7);

echo "====== object with index; ++/-- =========\n";

var_dump($c10[1000]++);
echo "------\n";
var_dump(--$c10[1000]);

echo "====== object without index; simple assignment =========\n";

var_dump($c10[] = 987);
echo "------\n";

var_dump($c10[] = TRUE);
echo "------\n";

var_dump($c10[] = 'xyz');

echo "====== object without index; compound assignment =========\n";

var_dump($c10[] += 5);
echo "------\n";

var_dump($c10[] -= 5);
echo "------\n";

var_dump($c10[] *= 5);

echo "====== object without index; ++/-- =========\n";

var_dump($c10);
echo "------\n";

var_dump($c10[]++);
echo "------\n";
var_dump(++$c10[]);
echo "------\n";

var_dump(--$c10[]);

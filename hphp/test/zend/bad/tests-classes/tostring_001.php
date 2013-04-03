<?php

function my_error_handler($errno, $errstr, $errfile, $errline) {
	var_dump($errstr);
}

set_error_handler('my_error_handler');

class test1
{
}

class test2
{
    function __toString()
    {
    	echo __METHOD__ . "()\n";
        return "Converted\n";
    }
}

class test3
{
    function __toString()
    {
    	echo __METHOD__ . "()\n";
        return 42;
    }
}
echo "====test1====\n";
$o = new test1;
print_r($o);
var_dump((string)$o);
var_dump($o);

echo "====test2====\n";
$o = new test2;
print_r($o);
print $o;
var_dump($o);
echo "====test3====\n";
echo $o;

echo "====test4====\n";
echo "string:".$o;

echo "====test5====\n";
echo 1 . $o;
echo 1 , $o;

echo "====test6====\n";
echo $o . $o;
echo $o , $o;

echo "====test7====\n";
$ar = array();
$ar[$o->__toString()] = "ERROR";
echo $ar[$o];

echo "====test8====\n";
var_dump(trim($o));
var_dump(trim((string)$o));

echo "====test9====\n";
echo sprintf("%s", $o);

echo "====test10====\n";
$o = new test3;
var_dump($o);
echo $o;

?>
====DONE====
<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

include_once 'Point.inc';

echo "--------- TRUE -------------\n";

var_dump(empty(TRUE));
$v = TRUE;
var_dump(empty($v));

echo "--------- FALSE -------------\n";

var_dump(empty(FALSE));
$v = FALSE;
var_dump(empty($v));

echo "--------- -10 -------------\n";

var_dump(empty(-10));
$v = -10;
var_dump(empty($v));

echo "---------- 0 ------------\n";

var_dump(empty(0));
$v = 0;
var_dump(empty($v));

echo "---------- 123 ------------\n";

var_dump(empty(123));
$v = 123;
var_dump(empty($v));

echo "--------- -10.56 -------------\n";

var_dump(empty(-10.56));
$v = -10.56;
var_dump(empty($v));

echo "--------- 0.0 -------------\n";

var_dump(empty(0.0));
$v = 0.0;
var_dump(empty($v));

echo "-------- 123.456 --------------\n";

var_dump(empty(123.456));
$v = 123.456;
var_dump(empty($v));

echo "--------- NULL -------------\n";

var_dump(empty(NULL));
$v = NULL;
var_dump(empty($v));

echo "---------- \"\" ------------\n";

var_dump(empty(""));
$v = "";
var_dump(empty($v));

echo "---------- \"0\" ------------\n";

var_dump(empty("0"));
$v = "0";
var_dump(empty($v));

echo "---------- \"00\" ------------\n";

var_dump(empty("00"));
$v = "00";
var_dump(empty($v));

echo "--------- \"Hello\" -------------\n";

var_dump(empty("Hello"));
$v = "Hello";
var_dump(empty($v));

echo "--------- [] -------------\n";

var_dump(empty([]));
$v = [];
var_dump(empty($v));

echo "---------- [10, 20] ------------\n";

var_dump(empty([10, 20]));
$v = [10, 20];
var_dump(empty($v));

echo "--------- Point(3, 5) -------------\n";

$v = new Point(3, 5);
var_dump(empty($v));

echo "--------- instance of class having no properties -------------\n";

class XX {}
$v = new XX;
var_dump(empty($v));

echo "--------- undefined parameter -------------\n";

function f($p)
{
    var_dump($p);
    var_dump(empty($p));
}

f();
f(NULL);
f(10);

echo "---------- resource STDIN ------------\n";

var_dump(empty(STDIN));
$v = STDIN;
var_dump(empty($v));

echo "---------- dynamic property ------------\n";

class X1
{
}

class X2
{
    public function __isset($name)
    {
        echo "Inside " . __METHOD__ . " with \$name $name\n";
//      return FALSE;
        return TRUE;
    }
}

$x1 = new X1;
var_dump(empty($x1->m));
$x1->m = 123;
var_dump(empty($x1->m));

$x2 = new X2;
var_dump(empty($x2->m));

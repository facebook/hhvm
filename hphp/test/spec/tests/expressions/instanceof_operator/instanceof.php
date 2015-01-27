<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
echo "------- Classes -------------\n";

// test using a series of classes, some derived

class C1 {}
class C2 {}
class D extends C1 {}

$c1 = new C1;
$c2 = new C2;
$c2b = new C2;
$d = new D;

var_dump($c1 instanceof C1);
var_dump($c1 instanceof C2);
var_dump($c1 instanceof D);

//var_dump($c1 instanceof "C1");    // can't be a string literal
//var_dump($c1 instanceof "c1");    // can't be a string literal
$clName = "C1";
var_dump($c1 instanceof $clName);   // TRUE; can be a string
$clName = "C2";
var_dump($c1 instanceof $clName);   // FALSE; can be a string

var_dump($c2 instanceof $c2b);      //
var_dump($d instanceof $c1);        //
var_dump($c1 instanceof $d);        //

echo "--------------------\n";

function f1() { return new D; }
var_dump(f1() instanceof C1);
var_dump(f1() instanceof C2);
var_dump(f1() instanceof D);

echo "--------------------\n";

var_dump($c2 instanceof C1);
var_dump($c2 instanceof C2);
var_dump($c2 instanceof d);

echo "--------------------\n";

var_dump($d instanceof C1);
var_dump($d instanceof C2);
var_dump($d instanceof d);

echo "------- Interfaces -------------\n";

// test using a series of interfaces

interface I1 {}
interface I2 {}
class E1 implements I1, I2 {}

$e1 = new E1;

var_dump($e1 instanceof E1);
var_dump($e1 instanceof I1);    // Yes
var_dump($e1 instanceof I2);    // Yes
$iName = "I2";
var_dump($e1 instanceof $iName);    // Yes
//var_dump($e1 instanceof "I2");    // No string literal allowed

echo "------- Non-Instances -------------\n";

// test using variables that are not instances

var_dump($d instanceof I1);     // of course not!
$v = 10;
var_dump($v instanceof C1);     // of course not!
$v = 1.234;
var_dump($v instanceof C1);     // of course not!
$v = NULL;
var_dump($v instanceof C1);     // of course not!

echo "------- Non-class/Non-interface types -------------\n";

// test against non-class/non-interface "types" and even non-types

$v = TRUE;
var_dump($v instanceof bool);       // false!
var_dump($v instanceof int);        // false!
var_dump($v instanceof float);      // false!
var_dump($v instanceof string);     // false!
//var_dump($v instanceof array);    // won't parse
var_dump($v instanceof object);     // false!
var_dump($v instanceof xyz);        // false!
//*/

class C11 {} $c11 = new C11;
class C21 {} $c21 = new C21;
class D1 extends C11 {} $d = new D1;
var_dump($d instanceof C11);
var_dump($d instanceof C21);
var_dump($d instanceof D1);

// -----------------------------------------

interface I11 {}
interface I21 {}

class E11 implements I11, I21 {}

$e11 = new E11;
var_dump($e11 instanceof I11);  // Yes
var_dump($e11 instanceof I21);  // Yes
$iName = "I21";
var_dump($e11 instanceof $iName);   // Yes

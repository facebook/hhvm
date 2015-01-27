<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class C1 {}
class C2 {}

$c1a = new C1; // var_dump($c1a);
$c1b = new C1; // var_dump($c1b);
$c2 = new C2;  // var_dump($c2);

echo "\n===== compare instances of different object types =====\n\n";

var_dump($c1a >= $c2);  // bool(false)
var_dump($c1a >  $c2);  // bool(false)
var_dump($c1a <= $c2);  // bool(false)
var_dump($c1a <  $c2);  // bool(false)

echo "\n===== compare instances of the same (empty) object type =====\n\n";

var_dump($c1a >= $c1b); // bool(true)
var_dump($c1a >  $c1b); // bool(false)
var_dump($c1a <= $c1b); // bool(true)
var_dump($c1a <  $c1b); // bool(false)

echo "\n===== compare instances of the same object type with same values =====\n\n";

class C3 { public $x; }

$c3a = new C3; $c3a->x = 5; // var_dump($c3a);
$c3b = new C3; $c3b->x = 5; // var_dump($c3b);

var_dump($c3a >= $c3b); // bool(true)
var_dump($c3a >  $c3b); // bool(false)
var_dump($c3a <= $c3b); // bool(true)
var_dump($c3a <  $c3b); // bool(false)

echo "\n===== compare instances of the same object type with diff values =====\n\n";

$c3b->x = 7; // var_dump($c3a); var_dump($c3b);

var_dump($c3a >= $c3b); // bool(false)
var_dump($c3a >  $c3b); // bool(false)
var_dump($c3a <= $c3b); // bool(true)
var_dump($c3a <  $c3b); // bool(true)

echo "\n===== compare instances of the same object type with a pair of diff values =====\n\n";

// comparison seems to be done in lexical order of property definition; swapping
// the order of y and x gives different relational op results

class C4 { public $y; public $x; }

$c4a = new C4; $c4a->x = 3; $c4a->y = 6; // var_dump($c4a);
$c4b = new C4; $c4b->x = 5; $c4b->y = 2; // var_dump($c4b);

var_dump($c4a >= $c4b); // bool(true)
var_dump($c4a >  $c4b); // bool(true)
var_dump($c4a <= $c4b); // bool(false)
var_dump($c4a <  $c4b); // bool(false)

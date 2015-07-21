<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// Try to include a non-existant file

$inc = include_once 'XXPositions.inc';
var_dump($inc);

echo "----------------------------------\n";

// Include an existing file

$inc = include 'Positions.inc';
//$inc = include_once 'Positions.inc';
var_dump($inc);

// Include an existing file. It doesn't matter if the first include was with/without
// _once; subsequent use of include_once returns true

$inc = include_once 'Positions.inc';
var_dump($inc);

var_dump(Positions\LEFT);
var_dump(Positions\TOP);

echo "----------------------------------\n";

///*
// Include Point.inc to get at the Point class type

$inc = include 'Point.inc';
var_dump($inc);

$p1 = new Point(10,20);
//*/

echo "----------------------------------\n";

// Include Circle.inc to get at the Circle class type, which in turn uses the Point type

$inc = include('Circle.inc');
var_dump($inc);

$p2 = new Point(5, 6);
$c1 = new Circle(9, 7, 2.4);

echo "----------------------------------\n";

// get the set of included files

print_r(get_included_files());

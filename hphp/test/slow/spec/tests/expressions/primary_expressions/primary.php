<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

//($i) = 100;
$i = 100;
var_dump(((($i) + (10))));

$a = [100, 200];
var_dump($a[0]);
var_dump(($a[0]));      // redundant grouping parens
var_dump(($a)[0]);      // redundant grouping parens

$z = [[2,4,6,8], [5,10], [100,200,300]];
var_dump($z[0][2]);
var_dump(($z[0])[2]);   // redundant grouping parens

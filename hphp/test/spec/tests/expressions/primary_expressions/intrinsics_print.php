<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

include_once 'Point.inc';

$v1 = TRUE;
$v2 = 123;
$v3 = 20.3E2;
$v4 = NULL;
$v5 = "Hello";
$v6 = new Point(3, 5);

print '>>' . $v1 . '|' . $v2 . "<<\n";
print ('>>' . $v1 . '|' . $v2 . "<<\n");
print (('>>') . ($v1) . ('|') . ($v2) . ("<<\n"));// outer parens are part of optional syntax
                                                 // inner ones are redundant grouping parens
print '>>' . $v3 . '|' . $v4 . '|' . $v5 . '|' . $v6 . "<<\n";

$v3 = "qqq{$v2}zzz";
var_dump($v3);
print "$v3\n";

//print array(10, 20);  // Array to string conversion

//class C {}
//print new C;  //Object of class C could not be converted to string

if (print "xx\n") ;
10 > 5 ? print "AA\n" : print "ZZ\n";

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

/* this is a C-style comment
...
... "#"" is just comment text
... "//"" is just comment text
...
that runs for several whole lines */

$a = 100;   // this is a C++-style comment, ... "#"" is just comment text
$b = 12;    # this is a Perl-style comment, ... "//"" is just comment text

/*..*/$c/*..*/=/*..*/567/*..*/;/*..*/   // some short C-style comments interspersed with tokens
var_dump($c);

$i = 2; $j = 3; $k = $i+++$j;   # $i++ + $j
var_dump($k);
$i = 2; $j = 3; $k = $i++ +$j;  # $i++ + $j
var_dump($k);
$i = 2; $j = 3; $k = $i+ ++$j;  # $i + ++$j
var_dump($k);
$i = 2; $j = 3; $k = $i+ + +$j; # $i + + +$j; several unary +'s
var_dump($k);

//$i = 2; $j = 3; $k = $i++++$j;    // correctly diagnosed; sees $i++ ++ +$j, which is invalid

$i = 2; $j = 3; $k = $i+++/*..*/++$j;   // comment separates tokens like whitespace
var_dump($k);

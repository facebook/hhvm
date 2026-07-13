<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

/* this is a C-style comment
...
... "#"" is just comment text
... "//"" is just comment text
...
that runs for several whole lines */

$a = 100;   // this is a C++-style comment, ... "#"" is just comment text
$b = 12;    // this is a Perl-style comment, ... "//"" is just comment text

/*..*/$c/*..*/=/*..*/567/*..*/;/*..*/   // some short C-style comments interspersed with tokens
var_dump($c);

$i = 2; $j = 3; $t = $i; $i++; $k = $t+$j;   // $i+++$j tokenizes as $i++ + $j
var_dump($k);
$i = 2; $j = 3; $t = $i; $i++; $k = $t +$j;  // $i++ +$j tokenizes as $i++ + $j
var_dump($k);
$i = 2; $j = 3; ++$j; $k = $i+ $j;  // $i+ ++$j tokenizes as $i + ++$j
var_dump($k);
$i = 2; $j = 3; $k = $i+ + +$j; // $i + + +$j; several unary +'s
var_dump($k);

//$i = 2; $j = 3; $k = $i++++$j;    // correctly diagnosed; sees $i++ ++ +$j, which is invalid

$i = 2; $j = 3; $t = $i; $i++; ++$j; $k = $t+/*..*/$j;   // comment separates tokens like whitespace: $i++ + ++$j
var_dump($k);
}

<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

$a = 10;
echo '$a = '.$a."\n";
++$a;
echo '$a = '.$a."\n";
$b = $a;
echo '$b = '.$b."\n";
}

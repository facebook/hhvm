<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
<<__EntryPoint>> function main(): void {
$a = $x ==> $x + 1;
var_dump($a);

$o = new stdClass;
var_dump($o);
unset($o);

$b = $x ==> $x + 3;
var_dump($b);
}

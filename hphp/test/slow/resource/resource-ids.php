<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
<<__EntryPoint>> function main(): void {
$f = fopen(__FILE__, 'r');
var_dump($f);
unset($f);

$f = fopen(__FILE__, 'r');
var_dump($f);
}

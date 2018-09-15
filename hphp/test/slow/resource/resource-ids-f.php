<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$f = fopen(__FILE__, 'r');
var_dump($f);
unset($f);

$f = fopen(__FILE__, 'r');
var_dump($f);

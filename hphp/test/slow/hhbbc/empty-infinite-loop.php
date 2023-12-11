<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo($x) :mixed{
  foreach ($x as $v) { while (true) {} }
}

<<__EntryPoint>>
function main_empty_infinite_loop() :mixed{
foo(__hhvm_intrinsics\launder_value(vec[]));

echo "DONE\n";
}

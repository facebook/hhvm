<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo($x) {
  foreach ($x as $v) { while (true) {} }
}

<<__EntryPoint>>
function main_empty_infinite_loop() {
foo(__hhvm_intrinsics\launder_value([]));

echo "DONE\n";
}

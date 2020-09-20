<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $b, $c) {
  return $a + $b + $c;
}
<<__EntryPoint>> function main_entry(): void {
for ($i=0; $i < 5; $i++) {
  var_dump(main(1, 2, 3));
  var_dump(main(1, 2, 3.5));
  var_dump(main(1, 2.5, 3));
  var_dump(main(1, 2.5, 3.5));
  var_dump(main(1.5, 2, 3));
  var_dump(main(1.5, 2, 3.5));
  var_dump(main(1.5, 2.5, 3));
  var_dump(main(1.5, 2.5, 3.5));
}
var_dump(main(true, true, true));
}

<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  $x = "the quick brown fox compressed the lazy dog with qlz\n";
  $cx = qlzcompress($x);
  $ux = qlzuncompress($cx);
  var_dump($x);
  var_dump($cx);
  var_dump($ux);
  // Not a valid compressed string.  Without QLZ_MEMORY_SAFE this
  // segfaults.
  $wx = qlzuncompress('99999'.str_repeat('p', 4000));
  var_dump($wx);
}

main();

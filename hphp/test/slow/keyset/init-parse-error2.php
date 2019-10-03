<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $b, $c) {
  $ks = keyset[$a, inout $b, inout $c];
  return $ks;
}

<<__EntryPoint>>
function main_init_parse_error2() {
main(1, 2, 3);
}

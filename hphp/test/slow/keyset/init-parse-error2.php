<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $b, $c) :mixed{
  $ks = keyset[$a, inout $b, inout $c];
  return $ks;
}

<<__EntryPoint>>
function main_init_parse_error2() :mixed{
main(1, 2, 3);
}

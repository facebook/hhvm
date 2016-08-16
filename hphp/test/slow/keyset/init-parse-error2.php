<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $b, $c) {
  $ks = keyset[$a, &$b, &$c];
  return $ks;
}
main(1, 2, 3);

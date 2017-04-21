<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $b, $c) {
  $v = vec[$a, &$b, &$c];
  return $v;
}
main(1, 2, 3);

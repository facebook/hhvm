<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type ST = shape(?'rtl' => bool, 'b' => int);
function testit(?ST $s):bool {
  $c = $s ?? shape();
  invariant(Shapes::keyExists($c, 'rtl'), 'error');
  if ($c['rtl']) {
    return true;
  }
  return false;
}

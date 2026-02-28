<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(string $f, string $g): void {
  /* HH_FIXME[4009] */
  $x = $f();
  $x->$g();
}

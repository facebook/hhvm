<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4030] */
function any() {
  return 2;
}
function testit(bool $b):int {
  $x = null;
  foreach (any() as $y) {
    if ($b) {
      $x = any();
      break;
    }
  }
  if (!$x) {
    return 2;
  }
  return 3;
}

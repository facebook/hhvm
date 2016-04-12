<?hh //strict

/**
 * Setting fields in a shape-like array with class constants from different
 * classes downgrades it to a map-like array.
 */

class C1 {
  const KEY = 'aaa';
}

class C2 {
  const KEY = 'aaa';
}

function test(): void {
  $a = array();
  $a[C1::KEY] = 4;
  $a[C2::KEY] = 'aaa';

  // $a should be a map-like array now
  hh_show($a);
}

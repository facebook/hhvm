<?hh //strict

/**
 * Setting fields in a shape-like array with both string and class constants
 * downgrades it to a map-like array.
 */

class C {
  const KEY = 'aaa';
}

function test(): void {
  $a = array();
  $a['aaa'] = 4;
  $a[C::KEY] = 'aaa';

  // $a should be a map-like array now
  hh_show($a);
}

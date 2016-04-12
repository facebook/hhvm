<?hh //strict

/**
 * Setting a non-static key in nested shape-like array downgrades it to a
 * map-like array.
 */

function test(string $key): void {
  $a = Vector {array()};
  $a[0]['k1'] = 4;
  $a[0]['k2'] = 'aaa';
  $a[0][$key] = 4;
  hh_show($a[0]);
  take_string($a[0]['k2']);
}

function take_string(string $_): void {}

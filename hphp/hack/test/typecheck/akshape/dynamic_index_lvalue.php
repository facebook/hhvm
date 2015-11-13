<?hh //strict

/**
 * Setting a non-static key in shape-like array downgrades it to a map-like
 * array.
 */

function test(string $key): void {
  $a = array();
  $a['k1'] = 4;
  $a['k2'] = 'aaa';
  $a[$key] = 4;
  hh_show($a);
  take_string($a['k2']);
}

function take_string(string $_): void {}

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
  // Note: if we ever plug the "heterogeneus array access returns Tany" hole
  // this needs to be updated to "take_string($a['k2'])" and start reporting an
  // error for this test to still test what it intends to.
  take_string($a['k1']);
}

function take_string(string $_): void {}

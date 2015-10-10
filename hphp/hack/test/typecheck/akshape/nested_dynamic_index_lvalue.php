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
  // Note: if we ever plug the "heterogeneus array access returns Tany" hole
  // this needs to be updated to "take_string($a[0]['k2'])" and start reporting
  // an error for this test to still test what it intends to.
  take_string($a[0]['k1']);
}

function take_string(string $_): void {}

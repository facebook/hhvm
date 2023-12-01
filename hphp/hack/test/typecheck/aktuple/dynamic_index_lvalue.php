<?hh

/**
 * Setting a non-static key in tuple-like array downgrades it to a vec-like
 * array.
 */

function test(int $key): void {
  $a = vec[4, 'aaa'];
  $a[$key] = 4;
  hh_show($a);
  take_string($a[1]);
}

function take_string(string $_): void {}

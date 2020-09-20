<?hh //strict

/**
 * Setting a non-static key in nested tuple-like array downgrades it to a
 * vec-like array.
 */

function test(int $key): void {
  $a = Vector { varray[4, 'aaa'] };
  $a[0][$key] = 4;
  take_string($a[0][1]);
}

function take_string(string $_): void {}

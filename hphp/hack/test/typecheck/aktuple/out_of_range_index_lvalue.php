<?hh //strict

/**
 * Setting an out-of range key in tuple-like array downgrades it to a vec-like
 * array.
 */

function test(): void {
  $a = varray[4, 'aaa'];
  $a[99] = 4;
  hh_show($a);
  take_string($a[1]);
}

function take_string(string $_): void {}

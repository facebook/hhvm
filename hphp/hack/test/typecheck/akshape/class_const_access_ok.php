<?hh //strict

/**
 * Shape-like arrays preserve element types when used with class constants
 * - usage with no errors.
 */

class C {
  const K1 = 'k1';
  const K2 = 'k2';
}

function test(): void {
  $a = array();
  $a[C::K1] = 4;
  $a[C::K2] = 'aaa';
  take_int($a[C::K1]);
  take_string($a[C::K2]);
}

function take_int(int $_): void {}

function take_string(string $_): void {}

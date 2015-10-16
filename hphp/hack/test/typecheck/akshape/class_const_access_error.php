<?hh //strict

/**
 * Shape-like arrays preserve element types when used with class constants
 * - usage that should report errors.
 */
class C {
  const K1 = 'k1';
  const K2 = 'k2';
}

function test(): void {
  $a = array();
  $a[C::K1] = 4;
  $a[C::K2] = 'aaa';

  take_string($a[C::K1]);
}

function take_string(string $_): void {}

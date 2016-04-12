<?hh //strict

/**
 * Nested shape-like arrays preserve element types - usage that should report
 * errors.
 */

function test(): void {
  $a = Vector {array()};
  $a[0]['k1'] = 4;
  $a[0]['k2'] = 'aaa';
  take_string($a[0]['k1']);
}

function take_string(string $_): void {}

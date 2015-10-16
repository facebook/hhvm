<?hh //strict

/**
 * Nested shape-like arrays preserve element types - usage with no errors.
 */

function test(): void {
  $a = Vector {array()};
  $a[0]['k1'] = 4;
  $a[0]['k2'] = 'aaa';
  take_int($a[0]['k1']);
  take_string($a[0]['k2']);
}

function take_int(int $_): void {}

function take_string(string $_): void {}

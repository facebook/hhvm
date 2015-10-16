<?hh //strict

/**
 * Shape-like arrays preserve element types - usage with no errors.
 */

function test(): void {
  $a = array();
  $a['k1'] = 4;
  $a['k2'] = 'aaa';
  take_int($a['k1']);
  take_string($a['k2']);
}

function take_int(int $_): void {}

function take_string(string $_): void {}

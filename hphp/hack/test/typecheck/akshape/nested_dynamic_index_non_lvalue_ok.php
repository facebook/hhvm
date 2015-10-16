<?hh //strict

/**
 * Accessing a nested shape-like array with non static key in non lvalue
 * position doesn't change the array type - usage with no errors.
 */

function test(string $key): void {
  $a = array();
  $a['k1'] = 4;
  $a['k2'] = 'aaa';
  $_ = $a[$key];
  take_int($a['k1']);
  take_string($a['k2']);
}

function take_int(int $_): void {}

function take_string(string $_): void {}

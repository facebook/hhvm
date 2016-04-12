<?hh //strict

/**
 * Accessing a nested shape-like array with non static key in non lvalue
 * position doesn't change the array type - usage that should report errors.
 */

function test(string $key): void {
  $a = Vector {array()};
  $a[0]['k1'] = 4;
  $a[0]['k2'] = 'aaa';
  $_ = $a[0][$key];
  take_string($a[0]['k1']);
}

function take_string(string $_): void {}

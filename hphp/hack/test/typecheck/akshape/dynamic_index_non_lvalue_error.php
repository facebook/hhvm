<?hh //strict

/**
 * Accessing a shape-like array with non static key in non lvalue position
 * doesn't change the array type - usage that should report errors.
 */

function test(string $key): void {
  $a = array();
  $a['k1'] = 4;
  $a['k2'] = 'aaa';
  $_ = $a[$key];
  take_string($a['k1']);
}

function take_string(string $_): void {}

<?hh //strict

/**
 * Accessing a tuple-like array with dynamic key in non lvalue position
 * doesn't change the array type - usage with no errors.
 */

function test(int $key): void {
  $a = varray[4, 'aaa'];
  $_ = $a[$key];

  take_int($a[0]);
  take_string($a[1]);
}

function take_int(int $_): void {}
function take_string(string $_): void {}

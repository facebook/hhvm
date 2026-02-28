<?hh

/**
 * Accessing a tuple-like array with dynamic key in non lvalue position
 * doesn't change the array type - usage that should report errors.
 */

function test(int $key): void {
  $a = vec[4, 'aaa'];
  $_ = $a[$key];

  take_string($a[0]);
}

function take_string(string $_): void {}

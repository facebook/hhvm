<?hh

/**
 * Test if the inner value type of AKvarray is correctly inferred
 */
function test(): void {
  $a = varray[];
  $a[] = 'aaa';
  // should error, because it's an array (used like a vector) containing strings
  f($a);
}

function f(varray<int> $_): void {}

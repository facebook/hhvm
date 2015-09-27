<?hh //strict

/**
 * Test AKempty to AKvec upgrade when inside a nested type
 */
function test(): void {
  $a = Vector {array()};
  $a[0][] = 'aaa';
  f($a);
}

function f(ConstVector<int> $_): void {}

<?hh //strict

/**
 * Test if AKempty local variable is upgraded to AKdarray after [] operation and
 * error message points to [] as the reason
 */
function test(): void {
  $a = varray[];
  $a[] = 'aaa';
  f($a);
}

function f(int $_): void {}

<?hh

/**
 * Test if AKempty local variable is upgraded to AKdarray after [] operation and
 * error message points to [] as the reason
 */
function test(): void {
  $a = vec[];
  $a[] = 'aaa';
  f($a);
}

function f(int $_): void {}

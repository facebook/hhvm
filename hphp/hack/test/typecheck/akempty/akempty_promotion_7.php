<?hh

/**
 * Test AKempty promotion if assignment happens inside an expression, not on
 * statement level.
 */
function test(): void {
  $a = vec[];
  if ($a[] = 'aaa') {

  }
  f($a);
}

function f(int $_): void {}

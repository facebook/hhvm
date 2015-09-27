<?hh //strict

/**
 * Test AKempty to AKvec upgrade when inside typevar and unresolved type
 */
function test(bool $b): void {
  $a = array();
  if ($b) {
  }
  hh_show($a); // should by unresolved type inside a typevar
  $a[] = 'aaa';
  f($a);
}

function f(int $_): void {}

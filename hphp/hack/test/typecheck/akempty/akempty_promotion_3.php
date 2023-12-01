<?hh

/**
 * Test AKempty to AKvarray upgrade when inside typevar and unresolved type
 */
function test(bool $b): void {
  $a = vec[];
  if ($b) {
  }
  hh_show($a); // should by unresolved type inside a typevar
  $a[] = 'aaa';
  f($a);
}

function f(int $_): void {}

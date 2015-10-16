<?hh //strict

/**
 * Test if AKempty local variable is upgraded to AKmap after [...] operation
 * and error message points to [...] as the reason
 */
function test(int $i): void {
  $a = array();
  $a[$i] = 'aaa';
  f($a);
}

function f(int $_): void {}

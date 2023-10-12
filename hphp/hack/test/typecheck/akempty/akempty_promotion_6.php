<?hh //strict

/**
 * Rvalue [...] usage should not cause AKempty upgrade
 */
function test(): void {
  $a = darray[];

  $a['aaa'];

  f($a);
  g($a);
}

function f(varray<int> $_): void {}
function g(darray<string, string> $_): void {}

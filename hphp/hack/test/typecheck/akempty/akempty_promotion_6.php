<?hh //strict

/**
 * Rvalue [...] usage should not cause AKempty upgrade
 */
function test(): void {
  $a = array();

  $a['aaa'];

  f($a);
  g($a);
}

function f(array<int> $_): void {}
function g(array<string, string> $_): void {}

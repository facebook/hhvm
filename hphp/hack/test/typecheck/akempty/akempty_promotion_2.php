<?hh //strict

/**
 * Test if the upgraded type is accepted with no errors by a compatible function
 */
function test(): void {
  $a = varray[];
  $a[] = 'aaa';
  f($a);
}

function f(array<string> $_): void {}

<?hh

/**
 * Test if the upgraded type is accepted with no errors by a compatible function
 */
function test(): void {
  $a = vec[];
  $a[] = 'aaa';
  f($a);
}

function f(varray<string> $_): void {}

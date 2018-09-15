<?hh // strict

function consumeArrayOfIntToString(array<int, string> $_): void {}

/**
 * Test confilicting AKempty promotions - in one branch it's promoted to
 * AKvec, in another to AKmap. Should be unresolved of those two afterwards.
 */
function test(): void {
  $array = array();
  if (true) {
    $array[] = '';
  } else {
    $array[0] = '';
  }
  consumeArrayOfIntToString($array);
}

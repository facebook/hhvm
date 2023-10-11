<?hh

function consumeArrayOfIntToString(darray<int, string> $_): void {}

/**
 * Test confilicting AKempty promotions - in one branch it's promoted to
 * AKvarray, in another to AKdarray. Should be unresolved of those two afterwards.
 */
function test(): void {
  $array = varray[];
  if (true) {
    $array[] = '';
  } else {
    $array[0] = '';
  }
  consumeArrayOfIntToString($array);
}

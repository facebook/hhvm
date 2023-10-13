<?hh

/**
 * Test confilicting AKempty promotions - in one branch it's promoted to
 * AKvarray, in another to AKdarray. Should be unresolved of those two afterwards.
 */
function test(bool $b): void {
  $a = varray[];
  if ($b) {
    $a[] = 'aaa';
  } else {
    $a[0] = 'aaa';
  }
  // no error - both vector and map like arrays match the darray<int, string>
  f($a);
}

function f(darray<int, string> $_): void {}

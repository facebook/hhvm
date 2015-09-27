<?hh //strict

/**
 * Test confilicting AKempty promotions - in one branch it's promoted to
 * AKvec, in another to AKmap. Should be unresolved of those two afterwards.
 */
function test(bool $b): void {
  $a = array();
  if ($b) {
    $a[] = 'aaa';
  } else {
    $a[0] = 'aaa';
  }
  // no error - both vector and map like arrays match the array<int, string>
  f($a);
}

function f(array<int, string> $_): void {}

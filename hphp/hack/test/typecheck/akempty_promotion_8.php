<?hh

/**
 * Test confilicting AKempty promotions - in one branch it's promoted to
 * AKvarray, in another to AKdarray. Should be unresolved of those two afterwards.
 */
function test(bool $b): void {
  $a = vec[];
  if ($b) {
    $a[] = 'aaa';
  } else {
    $a[0] = 'aaa';
  }
  // error - one of the branches result in a map-like array
  f($a);
}

function f(varray<string> $_): void {}

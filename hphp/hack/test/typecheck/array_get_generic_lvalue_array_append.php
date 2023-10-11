<?hh

/**
 * Appending to a vector-like array makes it unsafe to return it as original
 * generic type.
 */
function f<Tv as varray<mixed>>(Tv $a): Tv {
  $a[] = 4;
  return $a;
}

function test(varray<string> $a): varray<string> {
  return f($a);
}

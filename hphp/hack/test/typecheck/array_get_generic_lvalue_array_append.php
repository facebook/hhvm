<?hh //strict

/**
 * Appending to a vector-like array makes it unsafe to return it as original
 * generic type.
 */
function f<Tv as array<mixed>>(Tv $a): Tv {
  $a[] = 4;
  return $a;
}

function test(array<string> $a): array<string> {
  return f($a);
}

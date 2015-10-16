<?hh //strict

/**
 * Generic type remains generic if it was not modified.
 */
function f<Tv as KeyedContainer<mixed, mixed>>(Tv $a): Tv {
  $a['a'];
  return $a;
}

function test(array<string, string> $a): array<string, string> {
  return f($a);
}

<?hh //strict

/**
 * Assignment to array makes it unsafe to return it as original generic type
 */
function f<Tv as KeyedContainer<mixed, mixed>>(Tv $a): Tv {
  $a['a'] = 4;
  return $a;
}

function test(array<string, string> $a): array<string, string> {
  return f($a);
}

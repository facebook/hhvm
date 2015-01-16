<?hh // strict

function array_values<Tv>(Container<Tv> $input): array<Tv> {
  // UNSAFE
}

function test(KeyedContainer<string, int> $xs): array<int> {
  return array_values($xs);
}

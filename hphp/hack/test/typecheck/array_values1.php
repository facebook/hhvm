<?hh // strict

function test(KeyedContainer<string, int> $xs): array<int> {
  return array_values($xs);
}

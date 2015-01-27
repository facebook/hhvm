<?hh

function array_values<Tv>(Container<Tv> $input): array<Tv> {
  // UNSAFE
}

function test1(array<string> $xs): array<string> {
  return array_values($xs);
}

function test2(array<int, string> $xs): array<string> {
  return array_values($xs);
}

function test3(array<string, string> $xs): array<string> {
  return array_values($xs);
}

function test4(ConstSet<int> $xs): array<int> {
  return array_values($xs);
}

function test5(ConstMap<string, int> $xs): array<int> {
  return array_values($xs);
}

function test6(ConstVector<int> $xs): array<int> {
  return array_values($xs);
}

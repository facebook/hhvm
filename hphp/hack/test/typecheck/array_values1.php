<?hh // strict

function array_values<Tv>(Container<Tv> $input): array<Tv> {
  // UNSAFE
}

function test(
  KeyedContainer<string, int> $xs
): void {
  hh_show(array_values($xs));
}

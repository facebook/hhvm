<?hh // partial

function bar(): array<int, ?int> {
  return darray[123 => null, 456 => 789];
}

function idx2<Tk as arraykey, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index): ?Tv {
  return idx($collection, $index);
}

function foo(): ?int {
  $a = idx2(bar(), 123);
  return $a;
}

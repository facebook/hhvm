<?hh

function bar(): array<int, ?int> {
  return array(123 => null, 456 => 789);
}

function idx2<Tk, Tv>(?Indexish<Tk, Tv> $collection, ?Tk $index): ?Tv {
  return idx($collection, $index);
}

function foo(): int {
  $a = idx2(bar(), 123);
  if ($a !== null) {
    return $a;
  }
  return 345;
}

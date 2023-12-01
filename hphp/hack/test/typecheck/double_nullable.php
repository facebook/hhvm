<?hh

function bar(): darray<int, ?int> {
  return dict[123 => null, 456 => 789];
}

function idx2<Tk as arraykey, Tv>(?KeyedContainer<Tk, Tv> $collection, ?Tk $index): ?Tv {
  return idx($collection, $index);
}

function foo(): int {
  $a = idx2(bar(), 123);
  if ($a !== null) {
    return $a;
  }
  return 345;
}

<?hh // strict

function iterate10(array<mixed> $a): dynamic {
  for ($i = 0; $i < 10; $i += 1) {
    yield $a[$i];
  }
}

function same(array<mixed> $a): dynamic {
  yield from iterate10($a);
}

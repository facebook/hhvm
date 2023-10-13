<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function dictCoalesce2(dict<string, int> $d, ?int $b): int {
  return coalesce2(idx($d, 'a'), $b);
}

function coalesce2<Tr, Ta as Tr, Tb as Tr>(?Ta $a, Tb $b): Tr {
  return $a ?? $b;
}

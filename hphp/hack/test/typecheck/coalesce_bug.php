<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function coalesce3<Tr, Ta as Tr, Tb as Tr, Tc as Tr>(
  ?Ta $a,
  ?Tb $b,
  Tc $c,
): Tr {
  return $a ?? $b ?? $c;
}

class C {}
interface I {}
function genI(): ?I {
  return null;
}
function genC(bool $f, ?C $a, ?C $b, ?I $c): ?C {
  $r = coalesce3($f ? $a : null, $b, genI());
  return $r;
}

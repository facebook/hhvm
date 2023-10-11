<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface Pred<-T> { }
final class EqualsPred<Tv> implements Pred<Tv> {
}

function disj<Tv>(vec<Pred<Tv>> $predicates,
): Pred<Tv> {
  throw new Exception();
}

function equals<Tv>(Tv $value): EqualsPred<Tv> {
  throw new Exception();
}

function greaterThan<Tv>(Tv $value): Pred<Tv> {
  throw new Exception();
}

function expect(Pred<?int> $predicate): void {
}

function testit(): void {
  $p1 = greaterThan(23);
  $p2 = equals(0);
  $v = vec[$p1, $p2];
  $a = disj($v);
  expect($a);
}

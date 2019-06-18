<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface Pred<-T> { }
final class EqualsPred<Tv> implements Pred<Tv> {
}

/* HH_FIXME[4110] */
function disj<Tv>(vec<Pred<Tv>> $predicates,
): Pred<Tv> {
}
/* HH_FIXME[4110] */
function equals<Tv>(Tv $value): EqualsPred<Tv> {
}
/* HH_FIXME[4110] */
function greaterThan<Tv>(Tv $value): Pred<Tv> {
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

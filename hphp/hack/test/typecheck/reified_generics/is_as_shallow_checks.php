<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Erased<T> {}
class Reified<reify T> {}
class Both<Ta, reify Tb> {}

function erased_test(): void {
  "good" as Erased<_>;
  "bad"  as Erased<int>;
}

function reified_test(): void {
  "good" as Reified<_>;
  "good" as Reified<int>;
  "good" as Reified<Reified<int>>;
  "good" as Reified<Reified<Reified<Erased<int>>>>;
}

function both_test(): void {
  "good" as Both<_, _>;
  "good" as Both<_, int>;
  "bad"  as Both<int, _>;
  "good" as Both<_, Reified<int>>;
  "good" as Both<_, Erased<int>>;
}

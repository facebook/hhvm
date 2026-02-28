<?hh

class MyA<+Tco, -Tcontra, Tinv> {}

class MyB<-T1, T2> extends MyA<Sub, T1, T2> {}

class Super {}

class Sub extends Super {}

function takes_my_a(MyA<Sub, Super, Super> $_): void {}

function subtype_bad_invariant(MyB<Super, Sub> $x): void {
  takes_my_a($x);
}

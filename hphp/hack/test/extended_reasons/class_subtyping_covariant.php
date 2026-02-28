<?hh

class MyA<+Tco, -Tcontra, Tinv> {}

class MyC<+T1, T2> extends MyA<T1, Super, T2> {}

class Super {}

class Sub extends Super {}

function takes_my_a(MyA<Sub, Super, Super> $_): void {}

function subtype_bad_covariant(MyC<Super, Super> $x): void {
  takes_my_a($x);
}

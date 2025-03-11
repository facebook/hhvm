<?hh

final class FirstIsReified<reify T1, T2> {}
final class SecondIsReified<T1, reify T2> {}
final class NeitherReified<T1, T2> {}
final class BothReified<reify T1, reify T2> {}

function test(mixed $x): void {
  $_ = $x is FirstIsReified<_, _>;
  $_ = $x is FirstIsReified<int, int>; // should error
  $_ = $x is FirstIsReified<int, _>;
  $_ = $x is FirstIsReified<_, int>; // should error

  $_ = $x is SecondIsReified<_, _>;
  $_ = $x is SecondIsReified<int, int>; // should error
  $_ = $x is SecondIsReified<int, _>; // should error
  $_ = $x is SecondIsReified<_, int>;

  $_ = $x is NeitherReified<_, _>;
  $_ = $x is NeitherReified<int, int>; // should error
  $_ = $x is NeitherReified<int, _>; // should error
  $_ = $x is NeitherReified<_, int>; // should error

  $_ = $x is BothReified<_, _>;
  $_ = $x is BothReified<int, int>;
  $_ = $x is BothReified<int, _>;
  $_ = $x is BothReified<_, int>;
}

<?hh // strict

class C {
  const int X = 42;
}

function test<T as C>(classname<T> $c): void {
  $_ = $c::X;
}

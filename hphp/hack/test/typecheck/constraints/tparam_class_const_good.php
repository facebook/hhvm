<?hh

class C {
  const int X = 42;
}

function test<T as C>(class<T> $c): void {
  $_ = $c::X;
}

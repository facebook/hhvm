<?hh

class C {
  public static int $x = 42;
}

function test<T as C>(class<T> $c): void {
  $_ = $c::$x;
}

<?hh

class C {
  public static int $x = 42;
}

function test<T as C>(classname<T> $c): void {
  $_ = $c::$x;
}

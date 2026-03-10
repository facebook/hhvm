<?hh

class Bar {
  public static int $x = 0;
}

function test_class_get(): void {
  $t = tuple(42, "hi");
  list(Bar::$x, $b) = $t;
}

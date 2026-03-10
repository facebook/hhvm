<?hh

class Foo {
  public int $x = 0;
  public string $y = "";
}

function test_obj_get(): void {
  $obj = new Foo();
  $t = tuple(42, "hi");
  list($obj->x, $obj->y) = $t;
}

<?hh

trait T {
  <<__Const>>
  public int $x = 4;
}

class C {
  use T;
}

function F() {
  $c = new C();
  $c->x = 42;
}

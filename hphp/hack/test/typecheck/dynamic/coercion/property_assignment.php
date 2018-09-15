<?hh // strict

class C {
  public dynamic $x = 5;
}

function foo(C $c): void {
  $c->x = "hi";
}

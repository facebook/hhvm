<?hh

class C {
  public int $x = 0;
}

function takes_int(int $i): void {};

function f(C $c) : void {
  $c->x = true; // Type mismatch
  takes_int($c->x); // No type mismatch because $c->x : bool & int;
}

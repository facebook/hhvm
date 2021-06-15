<?hh

class C {
  public static int $x = 0;
}

function takes_int(int $i): void {};

function f() : void {
  C::$x = true; // Type mismatch
  takes_int(C::$x); // No type mismatch because C::x : bool & int;
}

<?hh // strict
/* Tests accessing callsites with type constants in generics */
abstract class Box {
  abstract const type T;
  public abstract function get() : this::T;
}

class IntBox extends Box {
  const type T = int;
  public function get() : this::T {
    return 0;
  }
}

class C {
  public static function foo<T1 as Box, T2> (T1 $x) : T2  where T2 = T1::T {
    return $x->get();
  }
}


function test() : string {
  $x = new IntBox();
  $y = C::foo($x); // y should have type int
  return $y;
}

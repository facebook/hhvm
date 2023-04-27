<?hh

class C {
  public static int $i = 42;
}

function f(): classname<C> { return C::class; }

function like_classname_c(): void {
  $c = f();
  $c::$i;
}

function classname_c(): void {
  $c = C::class;
  $c::$i;
}

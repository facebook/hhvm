<?hh

class C {
  public static function m(): void {}
}

function f(): classname<C> { return C::class; }

function like_classname_c(): void {
  $c = f();
  $c::m();
}

function classname_c(): void {
  $c = C::class;
  $c::m();
}

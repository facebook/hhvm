<?hh

class C {
  public static function m(): void {}
}

function f(): classname<C> { return C::class; }

function like_classname_c(): void {
  $c = f();
  $ptr = HH\classname_to_class($c);
  $ptr::m();
}

function classname_c(): void {
  $c = C::class;
  $ptr = HH\classname_to_class($c);
  $ptr::m();
}

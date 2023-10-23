<?hh

module z.z; // outside the active deployment

<<__EntryPoint>>
function main_3(): void {
  // functions
  foo();
  $f = "foo"; $f();
  __hhvm_intrinsics\launder_value("foo")();

  // classes
  new Foo();
  $c = "Foo"; new $c;
  $c = __hhvm_intrinsics\launder_value("Foo"); new $c;
  $c = __hhvm_intrinsics\launder_value(Foo::class); new $c;

  // methods
  Foo::foo();
  Foo::$f();
  $c::foo();
  $c::$f();
  __hhvm_intrinsics\launder_value(vec[$c, "foo"])();
  __hhvm_intrinsics\launder_value("Foo::foo")();
}

<?hh

module a.b; // in the active deployment

<<__EntryPoint>>
function main_2() : mixed {
  default_foo();
  __hhvm_intrinsics\launder_value("default_foo")();

  new DefaultFoo();

  $c = __hhvm_intrinsics\launder_value("DefaultFoo");
  new $c;

  DefaultFoo::static_foo();
  $c::static_foo();

  __hhvm_intrinsics\launder_value(vec[$c, "static_foo"])();
  __hhvm_intrinsics\launder_value("DefaultFoo::static_foo")();

  $c = __hhvm_intrinsics\launder_value(DefaultFoo::class);
  new $c;
}

<?hh

module a.b;

<<__EntryPoint>>
function main_soft() :mixed{
  soft_foo();

  __hhvm_intrinsics\launder_value("soft_foo")();

  new SoftFoo();

  $c = __hhvm_intrinsics\launder_value("SoftFoo");
  new $c;

  SoftFoo::static_foo();

  $c::static_foo();

  __hhvm_intrinsics\launder_value(vec[$c, "static_foo"])();
  __hhvm_intrinsics\launder_value("SoftFoo::static_foo")();

  var_dump($c::$static_foo);

  $c = __hhvm_intrinsics\launder_value(SoftFoo::class);
  new $c;
}

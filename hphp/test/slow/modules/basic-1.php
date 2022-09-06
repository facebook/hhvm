<?hh

<<file:__EnableUnstableFeatures("modules")>>

module A;

<<__EntryPoint>>
function main() {
  include 'basic-1.inc';
  Cls::foo_static();
  __hhvm_intrinsics\launder_value(new Cls)->foo();
  foo();
  __hhvm_intrinsics\launder_value("foo")();
  __hhvm_intrinsics\launder_value("Cls::foo_static")();
  __hhvm_intrinsics\launder_value(vec["Cls", "foo_static"])();
  __hhvm_intrinsics\launder_value(vec[new Cls, "foo"])();

  $x = __hhvm_intrinsics\launder_value("InternalCls");
  new $x;
  $x = __hhvm_intrinsics\launder_value(InternalCls::class);
  new $x;
  $y = __hhvm_intrinsics\launder_value("ReifiedInternalCls");
  new $y;
  InternalCls::foo_static();
  $x::foo_static();
  __hhvm_intrinsics\launder_value(vec[$x, "foo_static"])();
  __hhvm_intrinsics\launder_value("InternalCls::foo_static")();
}

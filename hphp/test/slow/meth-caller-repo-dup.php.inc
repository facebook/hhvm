<?hh

function other() {
  $mc = meth_caller(Foo::class, 'bar');
  $mc2 = __hhvm_intrinsics\launder_value(meth_caller(Foo::class, 'bar'));

  $mc(new Foo); $mc2(new Foo);
}

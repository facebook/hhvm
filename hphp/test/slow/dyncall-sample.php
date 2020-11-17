<?hh

<<__DynamicallyCallable(0)>> function foo() {}
<<__DynamicallyCallable(0)>> function bar() { echo "bar\n"; }

class Klass { <<__DynamicallyCallable(0)>> static function funktion() {} }
<<__DynamicallyConstructible(0)>> class Alpha {
  <<__DynamicallyCallable(0)>> function beta() { echo "Beta\n"; }
}

<<__EntryPoint>>
function main() {
  echo "Raw function call\n";
  foo(); bar();

  $ffun = foo<>;
  $bfun = bar<>;

  echo "Function pointer call\n";
  $ffun(); $bfun();

  $ffun2 = __hhvm_intrinsics\launder_value(foo<>);
  $bfun2 = __hhvm_intrinsics\launder_value(bar<>);

  echo "Function pointer call (laundered)\n";
  $ffun2(); $bfun2();

  $foo = __hhvm_intrinsics\launder_value('foo');
  $bar = __hhvm_intrinsics\launder_value('bar');

  echo "String Call (laundered)\n";
  $foo(); $bar();

  $foo2 = 'foo';
  $bar2 = 'bar';

  echo "String Call\n";
  $foo2(); $bar2();

  echo "Raw class method call\n";
  Klass::funktion();

  $cm = class_meth(Klass::class, 'funktion');

  echo "Class method pointer call\n";
  $cm();

  $cm2 = __hhvm_intrinsics\launder_value(class_meth(Klass::class, 'funktion'));

  echo "Class method pointer call (laundered)\n";
  $cm2();

  $klass = __hhvm_intrinsics\launder_value('klass');
  $funktion = __hhvm_intrinsics\launder_value('funktion');

  echo "Class/method string call (laundered)\n";
  Klass::$funktion();
  $klass::funktion();
  $klass::$funktion();

  $klass2 = 'klass';
  $funktion2 = 'funktion';

  echo "Class/method string call\n";
  Klass::$funktion2();
  $klass2::funktion();
  $klass2::$funktion2();

  $arr = varray[Klass::class, 'funktion'];

  echo "Class/method array call\n";
  $arr();

  $arr2 = __hhvm_intrinsics\launder_value(varray[Klass::class, 'funktion']);

  echo "Class/method array call (laundered)\n";
  $arr2();

  echo "Raw method call\n";
  (new Alpha)->beta();

  $alpha = __hhvm_intrinsics\launder_value('alpha');
  $beta = __hhvm_intrinsics\launder_value('beta');

  echo "Method string call (laundered)\n";
  (new Alpha)->$beta();
  (new $alpha)->beta();
  (new $alpha)->$beta();

  $alpha2 = 'alpha';
  $beta2 = 'beta';

  echo "Method string call\n";
  (new Alpha)->$beta2();
  (new $alpha2)->beta();
  (new $alpha2)->$beta2();

  $iarr = varray[new Alpha, 'beta'];

  echo "Method array call\n";
  $iarr();

  $iarr2 = __hhvm_intrinsics\launder_value(varray[new Alpha, 'beta']);

  echo "Method array call (laundered)\n";
  $iarr2();
}

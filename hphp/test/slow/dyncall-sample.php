<?hh

<<__DynamicallyCallable(0)>> function foo() :mixed{}
<<__DynamicallyCallable(0)>> function bar() :mixed{ echo "bar\n"; }

class Klass { <<__DynamicallyCallable(0)>> static function funktion() :mixed{} }
<<__DynamicallyConstructible(0)>> class Alpha {
  <<__DynamicallyCallable(0)>> function beta() :mixed{ echo "Beta\n"; }
}

<<__EntryPoint>>
function main() :mixed{
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

  $cm = Klass::funktion<>;

  echo "Class method pointer call\n";
  $cm();

  $cm2 = __hhvm_intrinsics\launder_value(Klass::funktion<>);

  echo "Class method pointer call (laundered)\n";
  $cm2();

  $klass = __hhvm_intrinsics\launder_value('Klass');
  $funktion = __hhvm_intrinsics\launder_value('funktion');

  echo "Class/method string call (laundered)\n";
  Klass::$funktion();
  $klass::funktion();
  $klass::$funktion();

  $klass2 = 'Klass';
  $funktion2 = 'funktion';

  echo "Class/method string call\n";
  Klass::$funktion2();
  $klass2::funktion();
  $klass2::$funktion2();

  $arr = vec[Klass::class, 'funktion'];

  echo "Class/method array call\n";
  $arr();

  $arr2 = __hhvm_intrinsics\launder_value(vec[Klass::class, 'funktion']);

  echo "Class/method array call (laundered)\n";
  $arr2();

  echo "Raw method call\n";
  (new Alpha)->beta();

  $alpha = __hhvm_intrinsics\launder_value('Alpha');
  $beta = __hhvm_intrinsics\launder_value('beta');

  echo "Method string call (laundered)\n";
  (new Alpha)->$beta();
  (new $alpha)->beta();
  (new $alpha)->$beta();

  $alpha2 = 'Alpha';
  $beta2 = 'beta';

  echo "Method string call\n";
  (new Alpha)->$beta2();
  (new $alpha2)->beta();
  (new $alpha2)->$beta2();

  $iarr = vec[new Alpha, 'beta'];

  echo "Method array call\n";
  $iarr();

  $iarr2 = __hhvm_intrinsics\launder_value(vec[new Alpha, 'beta']);

  echo "Method array call (laundered)\n";
  $iarr2();
}

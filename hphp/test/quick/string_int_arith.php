<?hh

function zero() :mixed{ return 0; }
function foo() :mixed{ return "0x10"; }
function twelve() :mixed{ return 12; }

function main() :mixed{
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()) / HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));

  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()));
  try {
    var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) / HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()));
  } catch (DivisionByZeroException $e) {}
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero()));

  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()) / HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()));

  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) / HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve()));
}

function setop_main() :mixed{
  $a = vec[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = vec[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = vec[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = vec[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);

  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
  var_dump($a[0]);
  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
  var_dump($a[0]);
  $a = vec[foo()];
  try {
    $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
    $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
    var_dump($a[0]);
  } catch (DivisionByZeroException $e) {}
  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
  var_dump($a[0]);

  $a = vec[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = vec[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = vec[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = vec[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);

  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
  $a = vec[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
}
<<__EntryPoint>> function main_entry(): void {
main();
setop_main();
}

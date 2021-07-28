<?hh

function zero() { return 0; }
function foo() { return "0x10"; }
function twelve() { return 12; }

function main() {
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

function setop_main() {
  $a = varray[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = varray[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = varray[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = varray[zero()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);

  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
  var_dump($a[0]);
  $a = varray[foo()];
  try {
    $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
    $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
    var_dump($a[0]);
  } catch (DivisionByZeroException $e) {}
  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(zero());
  var_dump($a[0]);

  $a = varray[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = varray[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = varray[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);
  $a = varray[twelve()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(foo());
  var_dump($a[0]);

  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] += HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] -= HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] /= HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
  $a = varray[foo()];
  $a[0] = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a[0]);
  $a[0] *= HH\Lib\Legacy_FIXME\cast_for_arithmetic(twelve());
  var_dump($a[0]);
}
<<__EntryPoint>> function main_entry(): void {
main();
setop_main();
}

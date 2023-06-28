<?hh
function t() :mixed{

  EvalOrder1514::$t++;
  return true;
}
function f() :mixed{

  EvalOrder1514::$f++;
  return false;
}
function i() :mixed{

  EvalOrder1514::$i++;
  return 1;
}
function d() :mixed{

  EvalOrder1514::$d++;
  return 3.14;
}
<<__EntryPoint>>
function foo() :mixed{
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(i() + i());
  var_dump(i() + d());
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()) + HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(d() + i());
  var_dump(d() + d());
  var_dump(EvalOrder1514::$t, EvalOrder1514::$f,           EvalOrder1514::$i, EvalOrder1514::$d);
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(i() - i());
  var_dump(i() - d());
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()) - HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(d() - i());
  var_dump(d() - d());
  var_dump(EvalOrder1514::$t, EvalOrder1514::$f,           EvalOrder1514::$i, EvalOrder1514::$d);
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(i()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(i() * i());
  var_dump(i() * d());
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(t()));
  var_dump(HH\Lib\Legacy_FIXME\cast_for_arithmetic(d()) * HH\Lib\Legacy_FIXME\cast_for_arithmetic(f()));
  var_dump(d() * i());
  var_dump(d() * d());
  var_dump(EvalOrder1514::$t, EvalOrder1514::$f,           EvalOrder1514::$i, EvalOrder1514::$d);
}

abstract final class EvalOrder1514 {
  public static $t = 0;
  public static $f = 0;
  public static $i = 0;
  public static $d = 0;
}

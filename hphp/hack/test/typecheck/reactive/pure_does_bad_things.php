<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Pure>>
function pure()[]: void {
  FooClass::$foo += 1;
  echo 'bar';
  $f = new FooClass();
  $f->bar++;
  $f2 = Rx\mutable(new FooClass());
  $f2->bar++;
}

<<__Rx>>
function rx()[rx]: void {
  FooClass::$foo += 1;
  echo 'bar';
  $f = new FooClass();
  $f->bar++;
  $f2 = Rx\mutable(new FooClass());
  $f2->bar++;
}

class FooClass {
  public static int $foo = 0;
  public int $bar = 42;
}

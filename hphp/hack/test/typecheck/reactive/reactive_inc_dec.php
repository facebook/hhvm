<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function rx()[]: void {
  $f = new FooClass();
  $f->bar++;
  ++$f->bar;
  $f->bar--;
  --$f->bar;
  $f2 = Rx\mutable(new FooClass());
  $f2->bar++;
  ++$f2->bar;
  $f2->bar--;
  --$f2->bar;
}

class FooClass {
  public int $bar = 42;
}

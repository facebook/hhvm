final class C {
  const type TWut = shape('a'=>int,'b'=>float,'c'=>bool);
}

function foo(C::TWut $_): void {}

function bar(
  shape('a'=>int,'b'=>float,...) $x,
  ) : void {

  foo(HH\FIXME\UNSAFE_CAST<shape(...),C::TWut>($x));
  foo(HH\FIXME\UNSAFE_CAST< shape('a'=>int,'b'=>float,...),C::TWut>($x));
}

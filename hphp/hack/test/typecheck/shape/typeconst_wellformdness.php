<?hh

abstract class C {
  abstract const type T as shape('a' => C::class, 'b' => int);
  abstract const type T2 as shape('a' => C::FOO, 'b' => int);

  abstract const type T3 as ?shape('a' => C::class, 'b' => int) = null;
  abstract const type T4 as ?shape('a' => C::FOO, 'b' => int) = null;
}
class D extends C {
  const type T = shape('a' => C::class, 'b' => int);
  const type T2 = shape('a' => C::FOO, 'b' => int);
}

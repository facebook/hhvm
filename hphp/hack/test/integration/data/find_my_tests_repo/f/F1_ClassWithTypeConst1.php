<?hh

class F1_ClassWithTypeConst1 {
  const type TBar = shape('foo' => F1_TypeAlias1);

  const type TBarShouldNotBeTraversed = shape('bla' => int);
}

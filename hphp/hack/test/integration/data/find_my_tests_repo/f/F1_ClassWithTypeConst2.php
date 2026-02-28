<?hh

class F1_ClassWithTypeConst2 {
  const type TBar = shape('foo' => F1_ClassWithTypeConst1::TBar);

  const type TBarShouldNotBeTraversed =
    shape('bla' => F1_ClassWithTypeConst1::TBarShouldNotBeTraversed);
}

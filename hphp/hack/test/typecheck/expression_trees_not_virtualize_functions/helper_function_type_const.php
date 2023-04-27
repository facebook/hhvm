<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Wrapper<T> {}

abstract class MyBox {
  abstract const type TInner as mixed;
}

class IntBox extends MyBox {
  const type TInner = ExampleInt;
}

async function setState<T as MyBox, TVal>(
  ExampleContext $_visitor,
): Awaitable<ExprTree<ExampleDsl, ExampleDsl::TAst, (function(Wrapper<T>, TVal): void)>>
where
  TVal = T::TInner {
  throw new \Exception();
}

function test(ExprTree<ExampleDsl, ExampleDsl::TAst, Wrapper<IntBox>> $x): void {
  ExampleDsl`setState(${$x}, 1)`;
}

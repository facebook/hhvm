<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class Wrapper<T> {}

abstract class MyBox {
  abstract const type TInner as mixed;
}

class IntBox extends MyBox {
  const type TInner = ExampleInt;
}

async function setState<T as MyBox with { type TInner = TVal }, TVal>(
  ExampleContext $_visitor,
): Awaitable<ExampleExpression<
  ExampleFunction<(function(Wrapper<T>, TVal): void)>,
>> {
  throw new \Exception();
}

function test(
  ExampleExpression<Wrapper<IntBox>> $x,
): void {
  ExampleDsl`setState(${$x}, 1)`;
}

<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class BC { }

type Action = Lambda<(function(int, BC): ExampleInt)>;

type Lambda<T> = ExampleDslExpression<ExampleFunction<T>>;

final class C {
  const type TAction = (function(BC): Awaitable<Action>);
  public function __construct(private this::TAction $action) {}
}

async function genAction(): Awaitable<C> {
    return new C(
      async (BC $a) ==> { return ExampleDsl`($_, BC $c) ==> 2`; },
    );
  }

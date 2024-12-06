<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>

async function g(): Awaitable<ExampleDslExpression<ExampleInt>> {
  return ExampleDsl`1`;
}

async function f(): Awaitable<void> {
  ExampleDsl`${ExampleDsl`${await g()}`}`;
  ExampleDsl`{
    ${await g()};
  }`;
  ExampleDsl`${
    ExampleDsl`{
      1;
      ${await g()};
    }`
  }`;
  ExampleDsl`{
    ${ExampleDsl`${await g()}`};
  }`;
  ExampleDsl`{
    ${await g()};
    ${ExampleDsl`${await g()}`};
  }`;
  ExampleDsl`{
    ${
      ExampleDsl`{
        ${await g()};
      }`
    };
  }`;
}

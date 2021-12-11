<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // Simple lambda
  ExampleDsl`(): ExampleVoid ==> {}`;

  // Lambda that returns a value that is not void
  ExampleDsl`(): ExampleInt ==> 1`;

  // Lambda that has a return already built in
  ExampleDsl`(): ExampleVoid ==> { return; }`;

  // Nested lambda that does not have a void return
  ExampleDsl`(): ExampleVoid ==> {
    (): ExampleInt ==> {
      return 1;
    };
  }`;

  // Multiple lambdas
  ExampleDsl`(): ExampleVoid ==> {
    (): ExampleVoid ==> {};
    (): ExampleVoid ==> { return; };
  }`;

  // Type correctness
  ExampleDsl`(ExampleBool $b): ExampleInt ==> {
    if ($b) {
      return 1;
    } else {
      return 2;
    }
  }`;

  // Type correct, despite the extra statement
  ExampleDsl`(): ExampleInt ==> {
    return 1;
    1;
  }`;

  // Type correct, despite the extra statement and the extra appended return
  ExampleDsl`(): ExampleVoid ==> {
    return;
    1;
  }`;
}

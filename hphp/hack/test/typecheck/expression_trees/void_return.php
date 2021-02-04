<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // Simple lambda
  Code`(): ExampleVoid ==> {}`;

  // Lambda that returns a value that is not void
  Code`(): ExampleInt ==> 1`;

  // Lambda that has a return already built in
  Code`(): ExampleVoid ==> { return; }`;

  // Nested lambda that does not have a void return
  Code`(): ExampleVoid ==> {
    (): ExampleInt ==> {
      return 1;
    };
  }`;

  // Multiple lambdas
  Code`(): ExampleVoid ==> {
    (): ExampleVoid ==> {};
    (): ExampleVoid ==> { return; };
  }`;

  // Type correctness
  Code`(ExampleBool $b): ExampleInt ==> {
    if ($b) {
      return 1;
    } else {
      return 2;
    }
  }`;

  // Type correct, despite the extra statement
  Code`(): ExampleInt ==> {
    return 1;
    1;
  }`;

  // Type correct, despite the extra statement and the extra appended return
  Code`(): ExampleVoid ==> {
    return;
    1;
  }`;
}

<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(): void {
  // We append a return; to the end of these but should not affect their correctness
  ExampleDsl`
    (ExampleBool $b): ExampleInt ==> {
      if ($b) {
        return 1;
      } else {
        return 2;
      }
    }
  `;

  ExampleDsl`
    (): ExampleInt ==> {
      return 1;
      1;
    }
  `;
}

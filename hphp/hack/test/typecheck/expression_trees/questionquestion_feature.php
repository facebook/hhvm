<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test_unsupported(): void {
  ExampleDsl`(?ExampleInt $x, ExampleInt $y) ==> {
    // Testing unstable feature gating error
    $x ?? $y;
  }`;
}

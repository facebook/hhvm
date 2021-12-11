<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function lift<T>(T $_): ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> {
  throw new Exception();
}

function test(): void {
  $x = 1;

  $_ = ExampleDsl`() ==> {
    ${lift($x + 1)};
    // Make sure that typing environment doesn't escape past splice
    $x + 1;
    return;
  }`;
}

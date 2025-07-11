<?hh
<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(?ExampleInt $x) ==> $x is null`;
  ExampleDsl`(?ExampleInt $x) ==> $x is nonnull`;
  ExampleDsl`(?ExampleInt $x) ==> $x is ExampleInt`;
}

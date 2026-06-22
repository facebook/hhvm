<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx) ==> $arr[$idx]`;
}

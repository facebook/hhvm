<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(ExampleInt $x, ExampleInt $y) ==> {
    $x & $y;
    $x | $y;
    $x ^ $y;
    $x << $y;
    $x >> $y;
    ~$x;
  }`;
}

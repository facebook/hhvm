<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(ExampleInt $x, ExampleInt $y) ==> {
    $x & $y;
    $x | $y;
    $x ^ $y;
    $x << $y;
    $x >> $y;
    ~$x;
  }`;
}

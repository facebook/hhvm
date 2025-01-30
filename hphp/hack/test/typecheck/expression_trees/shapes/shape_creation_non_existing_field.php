<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): void {
  ExampleDsl`{
    $f = (shape('x' => ExampleInt) $shape) ==> 3;
    $f(shape('x' => 2, 'z' => 'hi'));
  }`;
}

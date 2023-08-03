<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $f = () ==> {
    $one = ExampleDsl`1`;
    $one_splice = ExampleDsl`${$one}`;
    return $one;
  };
}

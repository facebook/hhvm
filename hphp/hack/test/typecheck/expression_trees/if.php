<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $if = Code`(ExampleBool $x) ==> { if($x) { return 1; } return 2; }`;

  $if_else = Code`(ExampleBool $x, ExampleBool $y) ==> {
    if ($x) {
      return 1;
    } else if ($y) {
      return 2;
    } else {
      return 3;
    }
  }`;
}

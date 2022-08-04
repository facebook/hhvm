<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  ExampleDsl`(int $x) ==> {
    switch ($x) {
    case 1:
      break;
    default:
      break;
    }
  }`;
}

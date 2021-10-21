<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(int $x) ==> {
    switch ($x) {
    case 1:
      break;
    default:
      break;
    }
  }`;
}

<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definitions so we don't get naming errors.
class Code {}

function foo(): void {
  $if = Code`($x) ==> { if($x) { return 1; } return 2; }`;

  $if_else = Code`($x, $y) ==> {
    if ($x) {
      return 1;
    } else if ($y) {
      return 2;
    } else {
      return 3;
    }
  }`;
}

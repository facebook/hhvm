<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definitions so we don't get naming errors.
class Code {}
function bar(mixed $_): int { return 1; }

function foo(): void {
  $addition = Code`1 + 2`;
  $fun_call = Code`bar("baz")`;
  $lambda = Code`($x) ==> $x`;

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

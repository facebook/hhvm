<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`() ==> {
    $x = 0;
    do {
      $x = $x + 1;
    } while ($x < 0);
  }`;
}

<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function test(): void {
  Code`(vec<int> $vi) ==> {
    $x = 0;
    foreach($vi as $i) {
      $x = $x + $i;
    }
  }`;
}

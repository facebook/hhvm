<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  Code`
    () ==> {
      $x = "Hello";

      "$x World!\n";
    }
  `;
}

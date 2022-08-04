<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  ExampleDsl`
    () ==> {
      $x = "Hello";

      "$x World!\n";
    }
  `;
}

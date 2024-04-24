<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>
<<file:__EnableUnstableFeatures('expression_tree_nest')>>

function f(): void {
  $y = ExampleDsl`${`1`}`; // Ok
  $y = ExampleDsl`${(() ==> {ExampleDsl2`1`; return ExampleDsl`2`;})()}`; // Ok
  $y = ExampleDsl2`${(() ==> {ExampleDsl2`1`; return ExampleDsl`2`;})()}`; // Error: type mismatch on DSLs
}

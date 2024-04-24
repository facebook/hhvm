<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('expression_tree_nest')>>

<<__EntryPoint>>
function f(): void {
  $y = ExampleDsl`1 + ${`2 + ${`3`}`}`; // Ok
  $y = ExampleDsl`1 + ${(() ==> { $x = ExampleDsl`1`; return ExampleDsl`2 + ${$x}`;})()}`; // ok
  $y = ExampleDsl`1 + ${(() ==> { $x = ExampleDsl`1`; return ExampleDsl`2 + $x`;})()}`; // error $x is unbound
}

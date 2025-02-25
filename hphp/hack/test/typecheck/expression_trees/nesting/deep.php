<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function f(): void {
  $y = ExampleDsl`1 + ${ExampleDsl`2 + ${ExampleDsl`3`}`}`; // Ok
  $y = ExampleDsl`1 + ${(() ==> { $x = ExampleDsl`1`; return ExampleDsl`2 + ${$x}`;})()}`; // ok
  $y = ExampleDsl`1 + ${(() ==> { $x = ExampleDsl`1`; return ExampleDsl`2 + $x`;})()}`; // error $x is unbound
}

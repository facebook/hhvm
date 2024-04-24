<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>
<<file: __EnableUnstableFeatures('expression_tree_nest')>>

<<__EntryPoint>>
function f(): void {
  $y = ExampleDsl`() ==> ${`1`}`; // Ok
  $y = ExampleDsl`() ==> ${ExampleDsl`1`}`; // Error: nested ET with DSL
  $y = ExampleDsl`() ==> {$x = 1; return ${`$x`};}`; // Error: unbound $x
  $y = ExampleDsl`() ==> {$x = 1; return ${$x};}`; // Error: unbound $x
  $y = ExampleDsl`() ==> {$x = 1; return $x;}`; // Ok
}

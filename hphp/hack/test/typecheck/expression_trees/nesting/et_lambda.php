<?hh
<<file: __EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function f(): void {
  $y = ExampleDsl`() ==> ${ExampleDsl`1`}`; // Ok
  $y = ExampleDsl`() ==> ${`1`}`; // Error: nested ET without DSL
  $y = ExampleDsl`() ==> {$x = 1; return ${ExampleDsl`$x`};}`; // Ok (nested with binding)
  $y = ExampleDsl`() ==> {$x = 1; return ${$x};}`; // Error: unbound $x
  $y = ExampleDsl`() ==> {$x = 1; return $x;}`; // Ok
}

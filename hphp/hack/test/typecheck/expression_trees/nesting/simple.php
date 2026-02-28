<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function f(): void {
  $y = ExampleDsl`${ExampleDsl`1`}`; // Ok
  $y = ExampleDsl2`${ExampleDsl2`1`}`; // Ok
  $y = (() ==> {ExampleDsl`1`; ExampleDsl2`1`; return 1;})(); // Ok
  $y = (() ==> {ExampleDsl2`1`; ExampleDsl`1`; return 1;})(); // Ok
  $y = ExampleDsl`${(() ==> {return ExampleDsl2`""`;})()}`; // Error: different name
  $y = ExampleDsl2`${(() ==> {return ExampleDsl`1`;})()}`; // Error: different name
}

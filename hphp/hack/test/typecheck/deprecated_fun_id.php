<?hh // partial

<<__Deprecated('use bar() instead')>>
function foo() {}

function f() {
  array_map(fun('foo'), varray[]);
}

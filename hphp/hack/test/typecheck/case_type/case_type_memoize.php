<?hh
<<file:__EnableUnstableFeatures('case_types')>>

class C {}

case type Good = int | bool | IMemoizeParam;

case type Bad = int | bool | C;

<<__Memoize>>
function f(Good $x): void {}

<<__Memoize>>
function g(Bad $x): void {}

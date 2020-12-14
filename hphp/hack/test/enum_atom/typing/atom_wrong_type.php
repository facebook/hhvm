<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

function f(<<__Atom>>int $x) : void {}
function g(<<__Atom>>string $x) : void {}

class C {}

function h(<<__Atom>>C $x) : void {}

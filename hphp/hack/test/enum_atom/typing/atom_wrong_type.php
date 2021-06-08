<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

function f(<<__ViaLabel>>int $x) : void {}
function g(<<__ViaLabel>>string $x) : void {}

class C {}

function h(<<__ViaLabel>>C $x) : void {}

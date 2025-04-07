<?hh
<<file:__EnableUnstableFeatures('simpli_hack')>>

function id<T>(T $x): T {
  return $x;
}

class FA implements HH\FunctionAttribute {
  public function __construct(public int $i = 4)[] {}
}

<<__SimpliHack(id(10), () ==> {})>> // OK
function f(int $i): mixed{}

<<FA(id(42))>> // ERROR cannot call functions in attributes
function g(): mixed{}

<<__EntryPoint>>
function main_simplihack() :mixed{
  f(10);
}

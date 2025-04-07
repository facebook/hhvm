<?hh
<<file:__EnableUnstableFeatures('simpli_hack')>>

function id<T>(T $x): T {
  return $x;
}

<<__SimpliHack(id(10), () ==> {
  return 'foo';
})>>
function f(int $i) :mixed{}

<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>


function f(int $i): int {
  let $j : int = $i; // compiles to $j = $i;
  return $j;
}

<<__EntryPoint>>
function main(): void {
  $x = f(3);
  var_dump($x);
}

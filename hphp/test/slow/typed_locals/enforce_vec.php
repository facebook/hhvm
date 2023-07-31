<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

<<__EntryPoint>>
function main(): void {
  let $x: vec<int>;
  $x = vec["a"];
  var_dump($x);
  $x[] = 1;
  var_dump($x);
  $x = dict[];
}

<?hh

<<file:__EnableUnstableFeatures('nameof_class')>>

<<__EntryPoint>>
function f(): void {
  $x = 4;
  var_dump(nameof $x);
}

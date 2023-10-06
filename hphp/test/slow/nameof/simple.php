<?hh

<<file:__EnableUnstableFeatures('nameof_class')>>

<<__EntryPoint>>
function main(): void {
  var_dump(nameof C); // deliberately undefined
}

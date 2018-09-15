<?hh

function sideEffects($x) {
  echo "side effects $x\n";
  return $x;
}


<<__EntryPoint>>
function main_set_literal_side_effects() {
var_dump(Set {sideEffects(1), sideEffects('a')});
}

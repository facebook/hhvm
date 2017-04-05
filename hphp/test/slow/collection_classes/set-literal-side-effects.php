<?hh

function sideEffects($x) {
  echo "side effects $x\n";
  return $x;
}

var_dump(Set {sideEffects(1), sideEffects('a')});

<?hh

class X { private function go($x) { return "this is a string"; } }
class N extends X { function go($x) { $z = 2; return $z; } }

function main(X $y) {
  $asd = 2;
  var_dump($y->go($asd));
}


<<__EntryPoint>>
function main_private_ancestor_003() {
main(new N);
}

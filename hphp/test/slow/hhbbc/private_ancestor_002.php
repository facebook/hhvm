<?hh

class X { private function go(inout $x) {} }
class N extends X {
  function __call($x, $y) { echo "ok\n"; }
}

function main(X $y) {
  $asd = 'asd';
  return $y->go($asd);
}


<<__EntryPoint>>
function main_private_ancestor_002() {
main(new N);
}

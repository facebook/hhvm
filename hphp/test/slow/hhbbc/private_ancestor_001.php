<?hh

class X { private function go(inout $x) :mixed{} }
class N extends X {
  function go($x) :mixed{ echo "ok\n"; }
}

function main(X $y) :mixed{
  $asd = 'asd';
  return $y->go($asd);
}


<<__EntryPoint>>
function main_private_ancestor_001() :mixed{
main(new N);
}

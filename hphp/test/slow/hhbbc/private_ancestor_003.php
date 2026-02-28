<?hh

class X { private function go($x) :mixed{ return "this is a string"; } }
class N extends X { function go($x) :mixed{ $z = 2; return $z; } }

function main(X $y) :mixed{
  $asd = 2;
  var_dump($y->go($asd));
}


<<__EntryPoint>>
function main_private_ancestor_003() :mixed{
main(new N);
}

<?hh

class X { private function go($x) :mixed{ return "this is a string"; } }
class N extends X { function go($x) :mixed{ return 0xbadf00d; } }

function main(X $y) :mixed{
  $asd = 2;
  var_dump($y->go($asd));
}


<<__EntryPoint>>
function main_method_resolution_004() :mixed{
main(new N);
}

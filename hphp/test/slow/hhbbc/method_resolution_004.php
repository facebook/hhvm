<?hh

class X { private function go($x) { return "this is a string"; } }
class N extends X { function go($x) { return 0xbadf00d; } }

function main(X $y) {
  $asd = 2;
  var_dump($y->go($asd));
}


<<__EntryPoint>>
function main_method_resolution_004() {
main(new N);
}

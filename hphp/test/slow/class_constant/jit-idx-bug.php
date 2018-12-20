<?hh

abstract class B {
  abstract const string NAME;
};

class C1 extends B {
  const string NAME = 'S1';
}

function test(dict $x, $c) {
  return idx($x, $c::NAME, "");
}

<<__EntryPoint>>
function main() {
  $d = dict['a' => 1, 'S1' => 2];
  $c1 = new C1();
  for ($i = 0; $i < 10; $i++) {
    test($d, $c1);
  }
  var_dump(test($d, $c1));
}

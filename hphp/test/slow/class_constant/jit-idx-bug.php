<?hh

abstract class B {
  abstract const string NAME;
}

class C1 extends B {
  const string NAME = 'S1';
  public static $prop = 'S2';
}

function test_clscns(dict $x, $c) :mixed{
  return idx($x, $c::NAME, "");
}

function test_cgets(dict $x, $c) :mixed{
  return idx($x, $c::$prop, "");
}

<<__EntryPoint>>
function main() :mixed{
  $d = dict['a' => 1, 'S1' => 2, 'S2' => 3];
  $c1 = new C1();
  for ($i = 0; $i < 10; $i++) {
    test_clscns($d, $c1);
    test_cgets($d, $c1);
  }
  var_dump(test_clscns($d, $c1));
  var_dump(test_cgets($d, $c1));
}

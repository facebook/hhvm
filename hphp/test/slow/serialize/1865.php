<?hh

class X {
  private $o, $a, $o2;
  function foo() :mixed{
    $this->o = $this;
    $this->a = vec[1,2,3];
    $this->o2 = $this;
  }
}
function test() :mixed{
  $x = new X;
  $x->foo();
  $s = serialize($x);
  var_dump($s);
  $y = unserialize($s);
  var_dump($y);
}

<<__EntryPoint>>
function main_1865() :mixed{
test();
}

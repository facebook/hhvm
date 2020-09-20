<?hh

class X {
  private $b = false;
  private $i = 0;
  private $a = varray[];
  private $s = 'hello';
  function set() {
    $this->b = true;
    $this->i = 5;
    $this->a = varray[1,2,3];
    $this->s = 'goodbye';
  }
  function foo() {
    var_dump($this->b, $this->i, $this->a, $this->s);
  }
}
function test() {
  $x = new X;
  $x->set();
  $s = serialize($x);
  $y = unserialize($s);
  $y->foo();
  var_dump($y);
}

<<__EntryPoint>>
function main_698() {
test();
}

<?hh

class X {
  private $str;
  private $arr;
  private $obj;
  function foo() {
    $this->str = 'hello';
    $this->arr = varray[1,2,3];
    $this->obj = $this;
  }
}
function test() {
  $x = new X;
  $s = serialize($x);
  $x = unserialize($s);
  var_dump($x);
}

<<__EntryPoint>>
function main_1550() {
test();
}

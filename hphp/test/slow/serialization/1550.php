<?hh

class X {
  private $str;
  private $arr;
  private $obj;
  function foo() :mixed{
    $this->str = 'hello';
    $this->arr = vec[1,2,3];
    $this->obj = $this;
  }
}
function test() :mixed{
  $x = new X;
  $s = serialize($x);
  $x = unserialize($s);
  var_dump($x);
}

<<__EntryPoint>>
function main_1550() :mixed{
test();
}

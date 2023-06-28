<?hh

class X {

  public function __invoke($x) :mixed{
    var_dump($x);
  }
  public function test() :mixed{
    $this(10);
    call_user_func($this, 300);
    call_user_func_array($this, varray[0, 1]);
  }
}
class Y {

  public function test($x) :mixed{
    $x(10);
    call_user_func($x, 300);
    call_user_func_array($x, varray[0, 1]);
  }
}

<<__EntryPoint>>
function main_771() :mixed{
$x = new X;
$x->test();
$y = new Y;
$y->test($x);
}

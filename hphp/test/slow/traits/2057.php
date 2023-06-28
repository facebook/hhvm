<?hh

trait T1 {
  abstract function bar():mixed;
  public function foo() :mixed{
    $this->bar();
  }
}

trait T2 {
  public function bar() :mixed{
    echo "Hello from bar()\n";
  }
}

class C {
  use T1, T2;
}




<<__EntryPoint>>
function main_2057() :mixed{
$o = new C;
$o->foo();
}

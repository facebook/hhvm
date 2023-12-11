<?hh

class A {
  public $a = 1;
}

class B extends A {
  public $m = 10;
  public function test() :mixed{
    $b = 'a';
    $this->$b = 'test';
    var_dump($this->$b);
    var_dump($this->a);
    $this->$b = vec[1];
    var_dump($this->a);
  }
}

 <<__EntryPoint>>
function main() :mixed{
  $obj = new B();
  $obj->test();
}

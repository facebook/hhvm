<?hh

class A {
  public $a = 1;
}

class B extends A {
  public $m = 10;
  public function test() {
    $b = 'a';
    $this->$b = 'test';
    var_dump($this->$b);
    var_dump($this->a);
    $this->$b = varray[1];
    var_dump($this->a);
  }
}

 <<__EntryPoint>>
function main() {
  $obj = new B();
  $obj->test();
}

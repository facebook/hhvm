<?hh

class A {
  private $a = varray['apple'];
  private $b = 'banana';
  static function foo() {
    $b = new A();
    unset($b->b);
    var_dump($b);
    foreach ($b as $prop => $value) {
      var_dump($prop);
    }
  }
}

<<__EntryPoint>>
function main_689() {
A::foo();
}

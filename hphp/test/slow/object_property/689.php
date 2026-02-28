<?hh

class A {
  private $a = vec['apple'];
  private $b = 'banana';
  static function foo() :mixed{
    $b = new A();
    unset($b->b);
    var_dump($b);
    foreach ($b as $prop => $value) {
      var_dump($prop);
    }
  }
}

<<__EntryPoint>>
function main_689() :mixed{
A::foo();
}

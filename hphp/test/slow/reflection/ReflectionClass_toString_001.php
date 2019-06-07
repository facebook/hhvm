<?hh

abstract class Foo {
  public $a;
  protected $b;
  private $c = 456;
  static protected $d = 789;

  abstract function bar($bar, $baz = 123);
  static final protected function baz(stdClass $a = null) {}
}


<<__EntryPoint>>
function main_reflection_class_to_string_001() {
echo (string)(new ReflectionClass('Foo'));
}

<?hh

abstract class Foo {
  public $a;
  protected $b;
  private $c = 456;
  static protected $d = 789;

  abstract function bar($bar, $baz = 123):mixed;
  static final protected function baz(stdClass $a = null) :mixed{}
}


<<__EntryPoint>>
function main_reflection_class_to_string_001() :mixed{
echo (string)(new ReflectionClass('Foo'));
}

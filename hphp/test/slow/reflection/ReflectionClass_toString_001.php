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
try {
  echo (string)(new ReflectionClass('Foo'));
} catch (TypecastException $e) {
  echo "TypecastException: ".$e->getMessage()."\n";
}
}

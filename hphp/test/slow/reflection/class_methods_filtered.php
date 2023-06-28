<?hh
abstract class A {
  private function f() :mixed{}
  abstract protected function g():mixed;
  abstract public function h():mixed;
}

interface I {
    public function i():mixed;
    public function j():mixed;
    static function s():mixed;
}

abstract class B extends A implements I {
  protected function g():mixed{}
  public function h():mixed{}
  public function j() :mixed{}
}


<<__EntryPoint>>
function main_class_methods_filtered() :mixed{
$ref = new ReflectionClass("B");
var_dump($ref->getMethods(ReflectionMethod::IS_ABSTRACT));
var_dump($ref->getMethods(ReflectionMethod::IS_STATIC));
}

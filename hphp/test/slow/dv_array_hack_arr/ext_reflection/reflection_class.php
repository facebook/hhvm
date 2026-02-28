<?hh

interface Foo {
  public function bar():mixed;
}

<<__Attr1(3, 4)>>
abstract class A implements Foo {
  const ONE = null;
  abstract const TWO;
  const type TFoo = mixed;
  protected $prop;
  private static $static_prop;
  public function baz() :mixed{}
}

trait TBing {
  require extends A;
  public function bong() :mixed{}
}


final class B extends A {
  use TBing;
  const TWO = null;
  public function bar() :mixed{}
  public function __construct($prop) {
    $this->prop = $prop;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a_cls = new ReflectionClass('A');
  var_dump($a_cls->getConstants());
  var_dump($a_cls->getAbstractConstantNames());
  $ordered_type_constants_meth =
    new ReflectionMethod('ReflectionClass::getOrderedTypeConstants');
  $ordered_type_constants_meth->setAccessible(true);
  var_dump($ordered_type_constants_meth->invoke(null, $a_cls->getName()));
  var_dump($a_cls->getTypeConstants());
  var_dump($a_cls->getDefaultProperties());
  var_dump($a_cls->getInterfaceNames());
  var_dump($a_cls->getInterfaces());
  var_dump($a_cls->getAttributes());
  var_dump($a_cls->getMethods());
  var_dump($a_cls->getProperties());
  var_dump($a_cls->getStaticProperties());
  $b_cls = new ReflectionClass('B');
  var_dump($b_cls->newInstanceArgs(vec[null]));
  var_dump($b_cls->getTraitNames());
  var_dump($b_cls->getTraits());
  $bing_cls = new ReflectionClass('TBing');
  var_dump($bing_cls->getRequirementNames());
  var_dump($bing_cls->getRequirements());
}

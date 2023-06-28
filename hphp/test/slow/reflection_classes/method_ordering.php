<?hh

trait A {
  abstract protected static function AAbsProtStat():mixed;

  private static function APrivStat1() :mixed{}
  private static function APrivStat2() :mixed{}

  public function APublic1() :mixed{}
  public function APublic2() :mixed{}
}

trait B {
  abstract protected static function BAbsProtStat():mixed;

  private static function BPrivStat1() :mixed{}
  private static function BPrivStat2() :mixed{}

  public function BPublic1() :mixed{}
  public function BPublic2() :mixed{}
}

class C {
  use B;
  use A;

  protected static function BAbsProtStat() :mixed{}
  protected static function AAbsProtStat() :mixed{}
}

class D extends C {
  public function foo() :mixed{}
}


<<__EntryPoint>>
function main_method_ordering() :mixed{
$cls = new ReflectionClass('D');
var_dump(array_map(
  function($meth) { return $meth->getName(); },
  $cls->getMethods()
));
}

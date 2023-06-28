<?hh

class A {}

abstract class Bar {
  protected ?string $name = null;
  public function getClassName() :mixed{
      return $this->name;
  }
}

final class Foo extends Bar {
  protected ?string $name = A::class;
}

<<__EntryPoint>>
function main() :mixed{
  $o = new Foo;
  var_dump($o->getClassName());
}

<?hh

class A {}

abstract class Bar {
  protected ?string $name = null;
  public function getClassName() {
      return $this->name;
  }
}

final class Foo extends Bar {
  protected ?string $name = A::class;
}

<<__EntryPoint>>
function main() {
  $o = new Foo;
  var_dump($o->getClassName());
}

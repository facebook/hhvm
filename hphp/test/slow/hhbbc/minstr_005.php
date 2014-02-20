<?hh

class SomethingUnrelated {
  public $name;
}

class Foo {
  private $name = "Foo";

  public function blah(SomethingUnrelated $heh) {
    $heh->name = 1024;
  }

  public function getName() { return $this->name; }
}

function main() {
  $x = new Foo();
  $x->blah(new SomethingUnrelated);
  var_dump($x->getName());
}
main();

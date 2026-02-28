<?hh

class SomethingUnrelated {
  public $name;
}

class Foo {
  private $name = "Foo";

  public function blah(SomethingUnrelated $heh) :mixed{
    $heh->name = 1024;
  }

  public function getName() :mixed{ return $this->name; }
}

function main() :mixed{
  $x = new Foo();
  $x->blah(new SomethingUnrelated);
  var_dump($x->getName());
}

<<__EntryPoint>>
function main_minstr_005() :mixed{
main();
}

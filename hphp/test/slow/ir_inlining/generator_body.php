<?hh

class CGetM {
  private $x;

  public function __construct() {
    $this->x = "asdasd";
  }

  function getX() :mixed{
    return $this->x;
  }

  public function genVarious() :AsyncGenerator<mixed,mixed,void>{
    $local = $this->getX();
    yield "a";
    yield $local;
    yield "c";
  }

  public function genInts($x) :AsyncGenerator<mixed,mixed,void>{
    yield $x;
    yield $x + 1;
  }
}

function foo() :AsyncGenerator<mixed,mixed,void>{
  $k = new CGetM();
  $z = $k->getX();
  yield $z;
  yield "\n";
}

function main() :mixed{
  foreach (foo() as $x) {
    echo $x;
  }

  $blah = new CGetM;
  foreach ($blah->genVarious() as $x) {
    echo $x;
  }

  $blah = new CGetM;
  foreach ($blah->genInts(666) as $y) {
    echo $y;
  }
}


<<__EntryPoint>>
function main_generator_body() :mixed{
main();
echo "\n";
}

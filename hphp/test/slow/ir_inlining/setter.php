<?hh

class Bar{
}

class Foo {
  private $x;
  private $y;

  public function __construct($x) {
    $this->x = $x;
  }

  public function setX($x) :mixed{
    $this->x = $x;
  }

  public function getX() :mixed{
    return $this->x;
  }

  public function setXVerified(string $str) :mixed{
    $this->x = $str;
  }

  public function setY(Bar $k) :mixed{
    $this->y = $k;
  }
}

function main() :mixed{
  $k = new Foo("something");
  echo $k->getX();
  echo "\n";
  $k->setX("foo");
  echo $k->getX();
  echo "\n";

  $k->setXVerified("string");
  echo $k->getX();
  echo "\n";

  $k->setY(new Bar);
}

<<__EntryPoint>>
function main_setter() :mixed{
main();
}

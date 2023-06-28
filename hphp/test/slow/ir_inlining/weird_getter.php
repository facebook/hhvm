<?hh

class WeirdGetter {
  private $x;

  public function __construct() {
    $this->x = "some string";
  }

  public function getX($ignored, $arg, $list, $lol) :mixed{
    return $this->x;
  }
}

function test8() :mixed{
  $k = new WeirdGetter();
  echo "blah" . $k->getX("string", "a", "string", "a");
  echo "\n";
}


<<__EntryPoint>>
function main_weird_getter() :mixed{
test8();
}

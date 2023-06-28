<?hh

class A {}
class B {}

class Foob {
  private $x;
  public function __construct() {
    $this->x = new B;
  }
}

function main(Foob $y) :mixed{
  echo "heh\n";
}


<<__EntryPoint>>
function main_repo_types_004() :mixed{
$l = "O:4:\"Foob\":1:{s:7:\"\000Foob\000x\";O:1:\"A\":0:{}}";
$y = unserialize($l);
var_dump($y);

main($y);
}

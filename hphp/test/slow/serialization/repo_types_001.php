<?hh

class Foob {
  private $x = false;
  public function __construct()[] {}
}

function main(Foob $y) :mixed{
  echo "heh\n";
}


<<__EntryPoint>>
function main_repo_types_001() :mixed{
$l = "O:4:\"Foob\":1:{s:7:\"\000Foob\000x\";s:5:\"heheh\";}";
$y = unserialize($l);
var_dump($y);

main($y);
}

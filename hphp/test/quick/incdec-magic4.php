<?hh

class Whatever {
  public function & __get($name) {
    var_dump($name); global $lol; return $lol;
  }
}

$lol = "asd";

function main() {
  $l = new Whatever();
  $l->blah++;
  var_dump($l);
}

main();
var_dump($lol);

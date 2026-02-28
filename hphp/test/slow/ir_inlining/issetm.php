<?hh

class IssetM {
  public function hasX() :mixed{
    return isset($this->x);
  }
}

function main() :mixed{
  $k = new IssetM();
  $v = $k->hasX();
  var_dump($v);
}


<<__EntryPoint>>
function main_issetm() :mixed{
main();
}

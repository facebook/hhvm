<?hh

class C {
  public function loadAllWithIDs($ids) {
    if (!count($ids = array_filter($ids))) {
      return varray[];
    }
    var_dump('muy malo', $ids);
    return -666;
  }
}

function main() {
  $testA = darray[4 => false, 5 => false];
  $c = new C();
  var_dump($c->loadAllWithIDs($testA));
}


<<__EntryPoint>>
function main_1326() {
main();
}

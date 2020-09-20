<?hh


function g($x) {
  return $x;
}

function h() {
  return varray[1,2];
}

class C {
  public $a;
  public $b;
  function f($x) {
    list($this->a, $this->b) = g($x) ? h() : tuple(null, null);
  }
}


<<__EntryPoint>>
function main() {
  $obj = new C;
  for ($i = 0; $i < 5; $i++) {
    $obj->f(!$i);
    var_dump($obj);
  }
}

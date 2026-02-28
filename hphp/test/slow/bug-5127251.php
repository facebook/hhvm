<?hh


function g($x) :mixed{
  return $x;
}

function h() :mixed{
  return vec[1,2];
}

class C {
  public $a;
  public $b;
  function f($x) :mixed{
    list($this->a, $this->b) = g($x) ? h() : tuple(null, null);
  }
}


<<__EntryPoint>>
function main() :mixed{
  $obj = new C;
  for ($i = 0; $i < 5; $i++) {
    $obj->f(!$i);
    var_dump($obj);
  }
}

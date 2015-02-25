<?hh


function g($x) {
  return $x;
}

function h() {
  return array(1,2);
}

class C {
  var $a;
  var $b;
  function f($x) {
    list($this->a, $this->b) = g($x) ? h() : tuple(null, null);
  }
}

$obj = new C;
for ($i = 0; $i < 5; $i++) {
  $obj->f(!$i);
  var_dump($obj);
}

<?hh

class X {
  function foo($x,$y) {
    $a = null;
    if ($x) {
      $a = new X;
    }
    new X($y ? null : $a);
    return $a;
  }
}

<<__EntryPoint>>
function main_1325() {
$x = new X;
var_dump($x->foo(false, true));
}

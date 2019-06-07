<?hh

class c {
  function x($y) {
    echo $y . "
";
    return $this;
  }
}
function p($x) {
  echo $x . "
";
  return $x;
}

<<__EntryPoint>>
function main_1509() {
$x = new c;
$x->x(3, p(1), p(2))->x(6, p(4), p(5));
}

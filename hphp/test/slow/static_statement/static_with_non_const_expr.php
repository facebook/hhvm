<?hh

function f() {
  $y = 1;
  static $x = $y;
}

f();

<?hh

function f($arg0, $arg1) {
 var_dump($arg0, $arg1);
 }
function g(inout $arg0, $arg1) {
 var_dump($arg0, $arg1);
 }
class Af {
  function g($f, $var) {
    $f($this, $var++);
  }
}
class Ag {
  function g($f, $var) {
    $thiz = $this;
    $f(inout $thiz, $var++);
  }
}

<<__EntryPoint>>
function main_1099() {
$af = new Af;
$ag = new Ag;
$af->g('f', 30);
$ag->g('g', 30);
}

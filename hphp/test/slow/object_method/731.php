<?hh

interface intf {
  function meth();
}
class m {
  function meth() {
    return 1;
  }
}
class m2 extends m implements intf {
}
class m3 extends m2 {
  function f() {
    var_dump(parent::meth());
  }
}
function g() {
  $y = new m3;
  var_dump($y->meth());
}

<<__EntryPoint>>
function main_731() {
$y = new m3;
$y->f();
g();
}

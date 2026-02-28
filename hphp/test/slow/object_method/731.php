<?hh

interface intf {
  function meth():mixed;
}
class m {
  function meth() :mixed{
    return 1;
  }
}
class m2 extends m implements intf {
}
class m3 extends m2 {
  function f() :mixed{
    var_dump(parent::meth());
  }
}
function g() :mixed{
  $y = new m3;
  var_dump($y->meth());
}

<<__EntryPoint>>
function main_731() :mixed{
$y = new m3;
$y->f();
g();
}

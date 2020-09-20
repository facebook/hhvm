<?hh
trait T2 {
  function F() {}
}
trait T3 {
  use T2 {
    F as G;
  }
}

<<__EntryPoint>>
function main_2606_simple() {
$rc3 = new ReflectionClass('T3');
var_dump($rc3->getTraitAliases());
}

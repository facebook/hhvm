<?hh

class B {
  function f4($arguments) {
    var_dump($arguments);
  }
}
class G extends B {
  function f4($a) {
    $b='B';
    $b::f4(5);
 // __call
  }
}

<<__EntryPoint>>
function main_1871() {
$g = new G(5);
$g->f4(3);
}

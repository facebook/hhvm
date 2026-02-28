<?hh

class B {
  function f4($arguments) :mixed{
    var_dump($arguments);
  }
}
class G extends B {
  function f4($a) :mixed{
    $b='B';
    $b::f4(5);
  }
}

<<__EntryPoint>>
function main_1871() :mixed{
$g = new G(5);
$g->f4(3);
}

<?hh

trait T {
  function f() :mixed{
    $a = function() {
     yield 1;
     yield 2;
    };
    return $a;
  }
}

class A {
  use T;
}

class B {
  use T;
}


<<__EntryPoint>>
function main_trait_generator_closure_001() :mixed{
$a = new A;
$f = $a->f();
foreach ($f() as $v) {
  var_dump($v);
}
}

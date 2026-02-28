<?hh

class Base {
  private function foo() :mixed{
    return 12;
  }
  function heh(D1 $d) :mixed{
    return $d->foo(); // calls the private function
  }
}

function gen($x) :mixed{
  return $x ? new D1 : null;
}

class D1 extends Base {
  public function foo() :mixed{
    return "a string";
  }
}

function main() :mixed{
  $x = new D1;
  var_dump($x->heh($x));
  $y = gen(1);
  var_dump($y->heh($y));
}


<<__EntryPoint>>
function main_method_resolution_002() :mixed{
main();
}

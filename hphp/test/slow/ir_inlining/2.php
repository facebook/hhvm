<?hh

class Test {
  public function foo($x) :mixed{
    return $x;
  }
}

function test4(Test $x) :mixed{
  $x->foo(12);
}


<<__EntryPoint>>
function main_2() :mixed{
test4(new Test());
}

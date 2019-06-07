<?hh

class Test {
  public function foo($x) {
    return $x;
  }
}

function test4(Test $x) {
  $x->foo(12);
}


<<__EntryPoint>>
function main_2() {
test4(new Test());
}

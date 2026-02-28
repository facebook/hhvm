<?hh

class C {}
class D extends C {}

class Test {
  public C $c;
  public function __construct() {
    //UNSAFE
  }
}
function test(Test $t): void {
  if ($t->c is D) {
    take_string($t->c);
  }
}

function take_string(string $s): void {}

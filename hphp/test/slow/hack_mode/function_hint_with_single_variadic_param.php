<?hh //strict

function mymeth(string... $y):void {
  var_dump($y);
}

class C {
  public function foo((function(...):void) $func) {
    $func("some", "string");
  }
}

$c = new C();
$c->foo(HH\fun('mymeth'));

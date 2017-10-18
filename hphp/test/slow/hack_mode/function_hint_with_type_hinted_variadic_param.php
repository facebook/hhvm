<?hh //strict

function mymeth(int $x, string... $y):void {
  var_dump($y);
}

class C {
  public function foo((function(int, string ...):void) $func) {
    $func(0, "some", "string");
  }
}

$c = new C();
$c->foo(HH\fun('mymeth'));

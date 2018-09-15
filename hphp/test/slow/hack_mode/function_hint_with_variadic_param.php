<?hh

function mymeth(int $x, ...$y):void {
  var_dump($y);
}

class C {
  public function foo((function(int, ...):void) $func) {
    $func(0, "some", "string");
  }
}


<<__EntryPoint>>
function main_function_hint_with_variadic_param() {
$c = new C();
$c->foo(HH\fun('mymeth'));
}

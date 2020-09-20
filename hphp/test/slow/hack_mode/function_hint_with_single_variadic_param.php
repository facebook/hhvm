<?hh

function mymeth(string... $y):void {
  var_dump($y);
}

class C {
  public function foo((function(string...):void) $func) {
    $func("some", "string");
  }
}


<<__EntryPoint>>
function main_function_hint_with_single_variadic_param() {
$c = new C();
$c->foo(HH\fun('mymeth'));
}

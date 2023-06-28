<?hh

function mymeth(int $x, string... $y):void {
  var_dump($y);
}

class C {
  public function foo((function(int, string ..., int):void) $func) :mixed{
    $func(0, "some", "string");
  }
}


<<__EntryPoint>>
function main_function_hint_with_params_trailing_variadic_param() :mixed{
$c = new C();
$c->foo(mymeth<>);
}

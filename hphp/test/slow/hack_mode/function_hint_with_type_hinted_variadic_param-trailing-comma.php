<?hh

function mymeth(int $x, string... $y):void {
  var_dump($y);
}

class C {
  public function foo((function(int, string ..., ):void) $func) :mixed{
    $func(0, "some", "string");
  }
}


<<__EntryPoint>>
function main_function_hint_with_type_hinted_variadic_param_trailing_comma() :mixed{
$c = new C();
$c->foo(mymeth<>);
}

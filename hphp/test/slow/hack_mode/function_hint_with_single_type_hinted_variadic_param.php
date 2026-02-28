<?hh

function mymeth(string... $y):void {
  var_dump($y);
}

class C {
  public function foo((function(string ...):void) $func) :mixed{
    $func("some", "string");
  }
}


<<__EntryPoint>>
function main_function_hint_with_single_type_hinted_variadic_param() :mixed{
$c = new C();
$c->foo(mymeth<>);
}

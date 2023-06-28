<?hh

function foo() :mixed{
  $x = new stdClass();
  $x->foo = "heh";
  return $x;
}

function bar() :mixed{
  var_dump(foo());
}




<<__EntryPoint>>
function main_minstr_002() :mixed{
bar();
}

<?hh

class a {
  protected $foo = 10;
}

<<__EntryPoint>>
function main_1817() :mixed{
$x = new a;
apc_store('x', vec[$x]);
$x = __hhvm_intrinsics\apc_fetch_no_check('x');
var_dump($x[0]);
}

<?hh

class a {
  protected $foo = 10;
}

<<__EntryPoint>>
function main_1817() {
$x = new a;
apc_store('x', varray[$x]);
$x = __hhvm_intrinsics\apc_fetch_no_check('x');
var_dump($x[0]);
}

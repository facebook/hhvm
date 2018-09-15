<?hh

function f() {
  $y = 1;
  static $x = $y;
}


<<__EntryPoint>>
function main_static_with_non_const_expr() {
f();
}

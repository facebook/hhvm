<?hh

class Foo {
  <<__DynamicallyCallable>>
  public static function add($x, $y) {
}
}

<<__EntryPoint>>
function main_23() {
$x = 0;
if (!call_user_func(varray['Foo', 'add'], $x, 0)) {
  echo 'foo';
}
}

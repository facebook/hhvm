<?hh

class Foo {
  <<__DynamicallyCallable>>
  public static function add($x, $y) :mixed{
}
}

<<__EntryPoint>>
function main_23() :mixed{
$x = 0;
if (!call_user_func(vec['Foo', 'add'], $x, 0)) {
  echo 'foo';
}
}

<?hh

interface I {
  static function foo();
}

<<__EntryPoint>>
function main() {
  $f = class_meth(I::class, 'foo');
  echo "FAIL\n";
  var_dump($f());
}

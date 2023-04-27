<?hh

interface I {
  static function foo();
}

<<__EntryPoint>>
function main() {
  $f = I::foo<>;
  echo "FAIL\n";
  var_dump($f());
}

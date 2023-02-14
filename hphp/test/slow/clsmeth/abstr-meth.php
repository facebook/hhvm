<?hh

abstract class C {
  abstract static function foo();
}

<<__EntryPoint>>
function main() {
  $f = C::foo<>;
  echo "FAIL\n";
  var_dump($f());
}

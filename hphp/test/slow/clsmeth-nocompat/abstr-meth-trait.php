<?hh

trait T {
  abstract static function foo();
}
abstract class C {
  use T;
}

<<__EntryPoint>>
function main() {
  $f = C::foo<>;
  echo "FAIL\n";
  var_dump($f());
}

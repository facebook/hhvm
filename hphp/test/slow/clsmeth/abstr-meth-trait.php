<?hh

trait T {
  abstract static function foo():mixed;
}
abstract class C {
  use T;
}

<<__EntryPoint>>
function main() :mixed{
  $f = C::foo<>;
  echo "FAIL\n";
  var_dump($f());
}

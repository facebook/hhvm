<?hh

abstract class C {
  abstract static function foo():mixed;
}

<<__EntryPoint>>
function main() :mixed{
  $f = C::foo<>;
  echo "FAIL\n";
  var_dump($f());
}

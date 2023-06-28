<?hh

interface I {
  static function foo():mixed;
}

<<__EntryPoint>>
function main() :mixed{
  $f = I::foo<>;
  echo "FAIL\n";
  var_dump($f());
}

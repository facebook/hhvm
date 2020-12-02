<?hh

abstract class C {
  abstract function foo();
}

<<__EntryPoint>>
function main() {
  $f = class_meth(C::class, 'foo');
  echo "FAIL\n";
  var_dump($f());
}

<?hh

class C { static function foo() { return class_meth('self', 'foo'); } }

<<__EntryPoint>>
function main() {
  C::foo();
  echo "FAIL\n";
}

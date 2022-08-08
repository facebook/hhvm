<?hh

abstract class A {}

class C extends A {
  static function foo() {
    echo "C::foo\n";
    parent::foo();
  }
}

<<__EntryPoint>>
function main() {
  C::foo();
  echo "done\n";
}

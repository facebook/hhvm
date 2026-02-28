<?hh

abstract class A {}

class C extends A {
  static function foo() :mixed{
    echo "C::foo\n";
    parent::foo();
  }
}

<<__EntryPoint>>
function main() :mixed{
  C::foo();
  echo "done\n";
}

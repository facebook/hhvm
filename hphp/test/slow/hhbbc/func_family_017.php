<?hh

class A {
  public function foo() :mixed{ return 1; }
}
abstract class B extends A {}
abstract class C extends A {}

<<__EntryPoint>>
function main() :mixed{
  echo "Done\n";
}

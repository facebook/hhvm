<?hh

module foo;
module newtype Foo = FooInternal; // ok
internal class FooInternal {
  public function foo(): void {
    echo "In function foo\n";
  }

}
function same_file(Foo $x) : void {
  $x->foo(); // ok
}

<<__EntryPoint>>
function main(): void {
  include "module_newtype_module.inc";
  include "module_newtype_separate_file.inc";
  include "module_newtype_outside_module.inc";

  $x = new FooInternal();
  same_file($x); // always ok
  separate_file($x); // always ok
  outside_module($x); // fails typechecker, but should pass at runtime
}

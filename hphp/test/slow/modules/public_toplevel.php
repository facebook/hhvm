<?hh

module foo;

public class Foo {}
public type FooAlias = Foo;
public enum FooEnum : int {}

public enum class FooEnumClass : int {}

<<__EntryPoint>>
public function foo(): void {
  include "public_toplevel.inc";
  echo "Good\n";
}

//// modules.php
<?hh


new module foo {}

//// test.php
<?hh


module foo;
public class Foo {}
public type FooAlias = Foo;
public enum FooEnum : int {}

public enum class FooEnumClass : int {}

<<__EntryPoint>>
  public function foo(): void {
    echo "Good\n";
  }

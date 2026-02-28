//// modules.php
<?hh


new module foo {}

//// test.php
<?hh

module foo;

internal newtype X = int;

internal enum Foo : int {
  HELLO = 1;

}

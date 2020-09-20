//// file1.php
<?hh // strict

newtype Foo = int;

//// file2.php
<?hh // strict

newtype Bar = string;

//// file3.php
<?hh // strict

function test(Foo $foo, Bar $bar): void {
  if ($foo === $bar) {
  }
}

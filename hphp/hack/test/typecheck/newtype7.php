//// newtype.php
<?hh // strict

newtype Foo as int = int;

//// useit.php
<?hh // strict

function f(Foo $x): ?Foo {
  return $x;
}

//// file1.php
<?hh // strict

newtype Foo = nonnull;

//// file2.php
<?hh // strict

abstract class Enum<T> {}

class Bar extends Enum<Foo> {}

//// file1.php
<?hh

trait S {
  public int $foo = 1;
}

//// file2.php
<?hh

abstract class C {
  <<__LateInit>>
  public int $foo;
}

//// file3.php
<?hh

final class D extends C {
  use S;
}

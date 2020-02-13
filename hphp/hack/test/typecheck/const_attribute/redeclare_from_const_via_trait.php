//// file1.php
<?hh

class C {
  <<__Const>> protected int $p = 1;
}

//// file2.php
<?hh

trait S {
  public int $p = 2;
}

//// file3.php
<?hh

class D extends C {
  use S;
}

//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    bar("expr1", "expr2", "expr3");
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    bar("__KSTAR_META_A");
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    /*REPLACE STMT*/bar("added arg", "__KSTAR_META_A", "another added arg");/**/
    return;
  }
}

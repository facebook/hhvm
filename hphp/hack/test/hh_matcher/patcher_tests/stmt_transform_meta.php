//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $bar = "Stmt1 true";
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $__ANY = "__ANY_META_A";
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    /*REPLACE STMT*/ echo "__META_A"; /**/
    return;
  }
}

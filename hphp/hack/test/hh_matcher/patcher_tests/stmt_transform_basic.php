//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    "Stmt1 true";
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__ANY";
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    /*REPLACE STMT*/ "A Different String"; /**/
    return;
  }
}

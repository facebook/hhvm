//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $foo = "Stmt1 true";
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $foo =
      "__ANY";
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $foo =
      /*REPLACE EXPR*/ "A Different String"; /**/
    return;
  }
}

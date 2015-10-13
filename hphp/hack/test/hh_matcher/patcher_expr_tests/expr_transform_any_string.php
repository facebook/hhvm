//// tosearch.php
<?hh // strict

class Foo {
  public function bar(): void {
    $foo = "Stmt1 true";
  }
}

//// matcherpattern.php
<?hh

$foo =
  "__ANY";

//// targetpattern.php
<?hh

$foo =
  /*REPLACE EXPR*/ "A Different String"; /**/

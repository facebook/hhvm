//// tosearch.php
<?hh // strict

class Foo {
  public function bar(): void {
    bar("expr1", "expr2", "expr3");
    bar("expr1");
    bar("expr1", "expr2");
    return;
  }
}

//// matcherpattern.php
<?hh

bar("__KSTAR");

//// targetpattern.php
<?hh

/*REPLACE EXPR*/bar("added arg", "another added arg");/**/

//// tosearch.php
<?hh // strict

class Foo {
  public function bar(): void {
    $bar = array(1, 2, 3);
    return;
  }
}

//// matcherpattern.php
<?hh

array(1, 2, 3);

//// targetpattern.php
<?hh

/*REPLACE EXPR*/Vector {1, 2, 3};/**/

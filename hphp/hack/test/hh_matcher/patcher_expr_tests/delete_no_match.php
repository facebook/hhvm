//// tosearch.php
<?hh // strict

class Foo {
  public function bar(): void {
    $a = 5;
    $a = $a * 10;
  }
}

//// matcherpattern.php
<?hh

$a + 10;

//// matchertarget.php
<?hh

/*DELETE EXPRS*/ /*end delete*/

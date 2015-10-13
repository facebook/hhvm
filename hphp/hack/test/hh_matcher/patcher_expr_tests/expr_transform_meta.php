//// tosearch.php
<?hh // strict

class Foo {
  public function bar(): void {
    $foo = array(1);
    return;
  }
}

//// matcherpattern.php
<?hh

$foo =
  array("__ANY_META_A");

//// targetpattern.php
<?hh

$foo =
  /*REPLACE EXPR*/Vector {"__META_A"};/**/

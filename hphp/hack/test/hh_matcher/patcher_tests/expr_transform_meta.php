//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $bar = array(1);
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $bar =
      array("__ANY_META_A");
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $bar->
      /*REPLACE EXPR*/Vector {"__META_A"};/**/
    return;
  }
}

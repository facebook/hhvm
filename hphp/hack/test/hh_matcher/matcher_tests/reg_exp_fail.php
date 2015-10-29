//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $a = "foo";
    $b = "bar";
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $a = "__REGEXP"; /*.*o.**/
    $b = "__REGEXP"; /*.*br.**/
  }
}

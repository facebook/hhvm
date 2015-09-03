//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $var = array ("val", "var", "more");
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $var = array ("val", "__KSTAR");
  }
}

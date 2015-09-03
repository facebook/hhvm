//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $bar = (true && false) && (true || false);
    $bar = ((true && false) && true || false);
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__KSTAR";
    $bar = true && false && true || false;
    "__KSTAR";
  }
}

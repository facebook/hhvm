//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    if (true) { }
    while (true) {
      echo "foo";
    }
    echo "bar";
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__ANY";
    "__ANY";
    echo "bar";
    return;
  }
}

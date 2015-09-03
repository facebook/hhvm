//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    if (true) {}
    while (true) {
      echo "things";
    }
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__KSTAR";
  }
}

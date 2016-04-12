//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    try {
      echo "foo";
    } catch (\Exception $e) {
      echo "bar";
    }
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    try {
      echo "foo";
    } catch (\Exception $__KSTAR) { }
  }
}

//// tosearch.php
<?hh //strict

  class Foo {
    public function bar(): void {
      if (true) {
        echo "a more";
      } else {
        echo "complicated stmt";
      }
      "another stmt";
      if (true) {
        echo "a more";
      } else {
        echo "complicated stmt";
      }
    }
  }

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__ANY_META_A";
    "__ANY";
    "__ANY_META_A";
  }
}

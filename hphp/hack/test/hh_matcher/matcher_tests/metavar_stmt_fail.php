//// tosearch.php
<?hh //strict

  class Foo {
    public function bar(): void {
      "a stmt";
      "another stmt";
      "a stmt2";
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

//// tosearch.php
<?hh //strict

  class Foo {
    public function bar(): void {
      $val = "an expr";
      "something random";
      echo "an expr";
    }
  }

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $val = "__ANY_META_A";
    "something random";
    echo "__META_A";
  }
}

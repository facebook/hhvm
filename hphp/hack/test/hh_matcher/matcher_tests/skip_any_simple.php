//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    while (true) {
      echo "foo";
    }
    return;
  }
}

//// matcherpattern.php
<?hh //strict


class Foo {
  public function bar(): void {
    "__SKIPANY";
    {
      echo "foo";
    }
    return;
  }
}

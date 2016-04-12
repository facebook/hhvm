//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    echo "hi";
    echo "bye";
    if (true) {
      echo "hi";
    }
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    echo "hi";
    echo "bye";
    if (true) {
      echo "hi";
    }
    return;
  }
}

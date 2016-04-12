//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    Baz::Foo();
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    Baz::Foo();
  }
}

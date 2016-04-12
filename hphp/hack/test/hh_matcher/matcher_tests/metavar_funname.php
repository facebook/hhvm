//// tosearch.php
<?hh //strict

  class Foo {
    public function bar(): void { }

    public function baz(): void {
      bar();
    }
  }

//// matcherpattern.php
<?hh //strict

class Foo {
  public function __SOMENAME_META_A(): void { }

  public function baz(): void {
    __META_A();
  }
}

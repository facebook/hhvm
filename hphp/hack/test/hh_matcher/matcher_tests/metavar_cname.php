//// tosearch.php
<?hh //strict

class Foo { }

class Bar {
  public function bar(): void {
    $f = new Foo();
  }
}

//// matcherpattern.php
<?hh //strict

class __SOMENAME_META_A { }

class Bar {
  public function bar(): void {
    $f = new __META_A();
  }
}

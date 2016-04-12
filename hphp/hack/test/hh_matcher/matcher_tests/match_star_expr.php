//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $foo = new Bar();
    $foo->baz("arg1", "arg2", "arg3");
    $foo->bar("arg1", "arg2", "arg3");
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    $foo = new Bar();
    $foo->baz("__KSTAR");
    $foo->__ANY("__KSTAR");
  }
}

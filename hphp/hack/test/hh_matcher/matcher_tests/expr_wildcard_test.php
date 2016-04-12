//// tosearch.php
<?hh //strict

class Foo {
  public function foo() : void {
    $var = new Bar();
    if (true) {
      $var->baz("argument");
      $var->foo(a1, a2);
    }
  }
}
//// matcherpattern.php
<?hh //strict

class Foo {
  public function foo() : void {
    $var = "__ANY";
    if (true) {
      $var->baz("__ANY");
      $var->__ANY("__KSTAR");
    }
  }
}

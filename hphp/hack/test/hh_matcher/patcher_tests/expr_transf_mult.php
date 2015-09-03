//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    baz("arg1");
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    baz(
      "arg1"
    );
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    baz(
      /*REPLACE EXPR*/ "arg2", "arg3" /**/
    );
    return;
  }
}

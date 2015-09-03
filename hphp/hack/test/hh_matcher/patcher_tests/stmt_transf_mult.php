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
    baz("arg1");
    return;
  }
}

//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    /*REPLACE STMT*/ baz("arg5"); bar("moreargs"); /**/
    return;
  }
}

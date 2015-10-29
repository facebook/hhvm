//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $foz = "Hello Alice";
    $foo = "Hello Bob";
    $bar = "Hello Carol";
    $baz = "Hi Dan";
    return;
  }
}

//// matcherpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__KSTAR";
    $__ANY = "__REGEXP"; /*Hello.**/
    "__KSTAR";
    return;
  }
}


//// targetpattern.php
<?hh //strict

class Foo {
  public function bar(): void {
    "__KSTAR";
    /*REPLACE STMT*/ $__ANY = "Good bye anon"; /**/
    return;
  }
}

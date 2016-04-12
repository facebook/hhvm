//// tosearch.php
<?hh //strict

class Bar {
  public function baz(): int {
    return 5;
  }
}

class Foo {
  public function bar(): void {
    $foo = $bar = 0;
    $foo++;
    --$foo;
    $c = $foo + 1;
    $c += d;
    $f = new Bar();
    $g = $f->baz();
  }
}

//// matcherpattern.php
<?hh //strict

class Bar {
  public function baz(): int {
    return 5;
  }
}

class Foo {
  public function bar(): void {
    $foo = $bar = 0;
    $foo++;
    --$foo;
    $c = $foo + 1;
    $c += d;
    $f = new Bar();
    $g = $f->baz();
  }
}

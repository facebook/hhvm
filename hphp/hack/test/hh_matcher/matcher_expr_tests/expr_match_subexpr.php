//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): bool {
    $a = true;
    $b = !$a;
    echo true;
    return $b && true;
  }
}

//// matcherpattern.php
<?hh

true;

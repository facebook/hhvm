//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): int {
    $a = (1 + 2) * 3;
    $a = 1 + 2 * 3;
    $a = (1 + (2 * 3));
    return $a;
  }
}

//// matcherpattern.php
<?hh

$a = ((1) + (2 * 3));

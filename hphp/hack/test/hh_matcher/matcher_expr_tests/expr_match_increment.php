//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): int {
    $a = 100;
    $a++;
    $b = 5;
    $a = $b - $a;
    $a++;
    return $a;
  }
}

//// matcherpattern.php
<?hh

$a++;

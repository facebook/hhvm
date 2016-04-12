//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $a = 5;
    $a++;
    $b = 6;
    $b++;
    $c = 10;
    $c++;
  }
}

//// matcherpattern.php
<?hh

$__ANY++;

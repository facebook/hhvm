//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): int {
    $a = 5;
    return $foo($a);
  }

  public function foo(int $c): int {
    echo $c;
    return $c;
  }
}

//// matcherpattern.php
<?hh

"__ANY";

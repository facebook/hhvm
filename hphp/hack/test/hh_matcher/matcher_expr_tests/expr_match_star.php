//// tosearch.php
<?hh //strict

class Foo {
  public function bar(): void {
    $this->f(5, 10, 15);
    $this->f(8, 8, 8);
    $this->g(5, 10);
  }

  public function f(int $a, int $b, int $c): int {
    return $a + $b + $c;
  }

  public function g(int $a, int $b): int {
    return $a * $b;
  }
}

//// matcherpattern.php
<?hh

$this->__ANY(5, "__KSTAR");

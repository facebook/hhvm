//// base-a.php
<?hh

trait A {
  require class C;

  public function foo(): int {
    return $this->bar();
  }
}
//// base-b.php
<?hh
final class C {
  use A;

  public function bar(): int {
    return 42;
  }
}

//// changed-a.php
<?hh

trait A {
  require class C;

  public function foo(): int {
    return $this->bar();
  }
}
//// changed-b.php
<?hh
final class C {
  use A;

  public function bar(): string {
    return "hello";
  }
}

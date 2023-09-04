//// base-a.php
<?hh

<<file:__EnableUnstableFeatures('require_class')>>

trait A {
  require class D;

  public function foo(): int {
    return $this->bar();
  }
}
//// base-b.php
<?hh

class C {
  public function bar(): int {
    return 42;
  }
}
//// base-c.php
<?hh

final class D extends C {
  use A;
}
//// changed-a.php
<?hh

<<file:__EnableUnstableFeatures('require_class')>>

trait A {
  require class D;

  public function foo(): int {
    return $this->bar();
  }
}
//// changed-b.php
<?hh

class C {
  public function bar(): string {
    return "hello";
  }
}
//// changed-c.php
<?hh

final class D extends C {
  use A;
}

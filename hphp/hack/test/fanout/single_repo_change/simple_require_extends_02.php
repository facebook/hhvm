//// base-a.php
<?hh
trait A {
  require extends C;

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
class D extends C { use A; }

//// changed-a.php
<?hh
trait A {
  require extends C;

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
class D extends C { use A; }

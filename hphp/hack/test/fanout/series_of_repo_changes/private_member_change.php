//// a.php
<?hh

class A {
  private function f(): void {}

  public function main(): void {
    $this->f();
  }
}

//// b.php
<?hh

class B extends A {
  public function f(int $x): int {
    return $x + 1;
  }
}

//// c.php
<?hh

class C extends A {}

//// main.php
<?hh

<<__EntryPoint>>
function main(): void {
  $b = new B();
  $b->main();
}


/////////////////////

//// a.php
<?hh

class A {
  private function f(string $_): void {}

  public function main(): void {
    $this->f("");
  }
}

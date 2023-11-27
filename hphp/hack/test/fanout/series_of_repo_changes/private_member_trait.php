//// a.php
<?hh

trait A {
  private function f(): void {}

  public function main(): void {
    $this->f();
  }
}

//// b.php
<?hh

class B {
  use A;

  public function f(): void {}
}

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

trait A {
  private function f(string $_): void {}

  public function main(): void {
    $this->f("");
  }
}

//// a.php
<?hh

trait A {
  private function f(): void {
    echo "from A\n";
  }

  public function main(): void {
    $this->f();
  }
}

//// b.php
<?hh

class B {
  use A;

  public function f(): void {
    echo "from B\n";
  }
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
  private function f(string $_): void {
    echo "from A\n";
  }

  public function main(): void {
    $this->f("");
  }
}

//// a.php
<?hh

class A {
  private function f(): void {
    echo "from A\n";
  }

  public function main(): void {
    $this->f();
  }
}

//// b.php
<?hh

class B extends A {
  public function f(int $x): int {
    echo "from B\n";
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
  private function f(string $_): void {
    echo "from A\n";
  }

  public function main(): void {
    $this->f("");
  }
}

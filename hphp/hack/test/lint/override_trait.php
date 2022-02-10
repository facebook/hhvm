<?hh

class C {
  public function f(): void {}
}

trait TCGood {
  require extends C;
  <<__Override>>
  public function f(): void {
    echo "In TCGood\n";
  }
}

trait TCBad {
  require extends C;

  public function f(): void {
    echo "In TCBad\n";
  }
}

interface I {
  public function g(): void;
}

trait TIGood {
  require implements I;
  public function g(): void {
    echo "In TIGood\n";
  }
}

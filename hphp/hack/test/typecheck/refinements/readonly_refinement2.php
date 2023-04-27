<?hh

class C {
  public arraykey $prop = 42;

  public function f(): void {
    if (readonly $this->prop is int) {
      takes_int($this->prop);
    }
  }
}

function takes_int(int $i): void {}

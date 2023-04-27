<?hh

interface I {}

class C implements I {
}

function f<T>(T $x) : void where T as bool {}

class D<T as I> {
  public function getT(): ?T { return null; }

  public function f(bool $b, dynamic $d) : void {
    if ($b) {
      $t = $d;
    } else {
      $t = $this->getT();
    }
    if ($t is C) {
      // This won't fail if the tpenv is inconsistent
      f(1);
    }
  }
}

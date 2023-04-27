<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class A<T> {
  public function __construct(private T $x) {}

  private function update(T $t) : T {
    $this->x = $t;
    return $t;
  }
  // Should be accepted
  public function f(B $b) : int {
    return $b->update(42);
  }
  // Should also be accepted
  public function fi(I $i) : int {
    return $i->update(42);
    }

  // should be rejected, T is not compatible with int
  public function g(B $b, T $t) : T {
    return $b->update($t);
  }
  // Also should be rejected, T is not compatible with int
  public function gi(I $i, T $t) : T {
    return $i->update($t);
  }

  public function get() : T { return $this->x; }
}

class B extends A<int> {}

interface I {
  require extends A<int>;
}

function expect_int(int $_): void {}

<<__EntryPoint>>
function main(): void {
  $a = new A("zuck");
  $b = new B(42);
  $a->g($b, "yolo");
  echo $b->get();
  echo "\n";
  expect_int($b->get()); // BOOM
  }

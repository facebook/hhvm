<?hh

class C {}

class A<T> extends C {
  public function __construct(private T $x) {}
  public function get(): T {
    return $this->x;
  }
  public function set(T $x): void {
    $this->x = $x;
  }
}
class B {}

function expect_arraykey(arraykey $_): void {}
function expect_int(int $_): void {}

function test1(arraykey $k): arraykey {
  $x = new A("");
  $y = $x->get();
  if ($y is arraykey) {
    // hh_show($y); // legacy: string, new infer: #28389
    expect_arraykey($y);
    expect_int($y); // legacy and new inf: error int incompatible with string
  } else {
    $y = $k;
  }
  return $y;
}

function test2(): string {
  $x = new A("");
  $y = $x->get();
  if ($y is B) {}
  // hh_show($y); // legacy: (string | B), new infer: (#28401 | B)
  return $y;
}

function test3(): void {
  $x = new C();
  if ($x is A<_>) {
    $x->set(""); // legacy and new inference: error string incompatible with T#1
    expect_int($x->get()); // legacy and new inference: error T#1 incompatible with int
  }
}

function test4(): void {
  $x = new A("");
  if ($x is A<_>) {
    expect_string($x->get());
    expect_int($x->get()); // legacy and new inference: error int incompatible with T#1
  }
}

function expect_string(string $_): void {}

function test5(arraykey $k): arraykey {
  $x = new A($k);
  $y = $x->get();
  if ($y is int) {
    expect_int($y);
  } else if ($y is string) {
    expect_string($y);
  }
  return $y;
}

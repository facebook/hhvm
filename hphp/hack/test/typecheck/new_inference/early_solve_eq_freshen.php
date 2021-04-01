<?hh

class A<T> {
  public function __construct(private T $x) {}
  public function get(): T {
    return $this->x;
  }
}

class B<+T> {
  public int $x = 0;
}

class C<+T> {}

function expect<T>(T $_) : void {}

function test(B<string> $b) : void {
  $x = new A($b); // Lower bound B<string>
  expect<B<arraykey>>($x->get()); // create shallow equal bound B
  expect<C<arraykey>>($x->get()); // incompatible upper bound C<arraykey>
  $x->get()->x; // expand type and solve to "freshened" shallow equal bound.
}

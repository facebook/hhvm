<?hh

class A<T> {
  public function __construct(private T $x) {}
  public function get(): T {
    return $this->x;
  }
}

class B<+T> {
  public function do_nothing(): void {}
}

class C<+T> {}

function expect<T>(T $_) : void {}

function test(B<string> $b) : void {
  $x = new A($b); // Lower bound B<string>
  expect<C<arraykey>>($x->get()); // incompatible upper bound C<arraykey>
  $x->get()->do_nothing(); // expand type and solve to lower bound.
}

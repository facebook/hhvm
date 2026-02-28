<?hh

function identity<T>(T $x): T { return $x; }

function pass_generic(): void {
  $f = identity<>;
  $_ = $f('a');
  $_ = $f(1);
  $_ = $f(shape('a' => 1, 'b' => 'hello'));
}

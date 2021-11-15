<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

enum class E : mixed {
  int Age = 42;
  string Name = "zuck";
}

function get_via_label<T>(<<__ViaLabel>>HH\MemberOf<E, T> $e) : T {
  return $e;
}

function get<T>(HH\MemberOf<E, T> $e): T { return $e; }

function expect_string(string $_): void {}
function expect_int(int $_): void {}

<<__EntryPoint>>
function good(): void {
  $f = ($x ==> get($x));
  expect_string($f(E::Name));

  echo $f(E::Name);
  echo "\n";
}

function bad(): void {
  $f = ($x ==> get($x));
  expect_int($f(E::Name)); // int vs string

  // Typing[4396] Labels are not allowed in this position. They are only
  // allowed in function call, if the function parameter is annotated with
  // __ViaLabel
  echo $f#Name();
  echo "\n";

  // wrong, we must pass a label to `get_via_label`
  $f = ($x ==> get_via_label($x));
  // same
  $f = ((HH\MemberOf<E, string> $x) ==> get_via_label($x));
  // more stuble, but wrong. The `__ViaLabel` automatically transform the input
  // label into a constant, so the $x with pass to get_via_label is wrong too
  $f = ((<<__ViaLabel>>HH\MemberOf<E, string> $x) ==> get_via_label($x));
  // bottom line, lambdas should use `HH\EnumClass\Label`, not __ViaLabel
}

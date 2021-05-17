<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

enum class E : mixed {
  int Age = 42;
  string Name = "zuck";
}

function with_atom<T>(<<__Atom>>HH\MemberOf<E, T> $e) : T {
  return $e;
}

function without_atom<T>(HH\MemberOf<E, T> $e): T { return $e; }

function expect_string(string $_): void {}
function expect_int(int $_): void {}

<<__EntryPoint>>
function good(): void {
  $f = ($x ==> without_atom($x));
  expect_string($f(E::Name));

  echo $f(E::Name);
  echo "\n";
}

function bad(): void {
  $f = ($x ==> without_atom($x));
  expect_int($f(E::Name)); // int vs string

  // Typing[4396] Atoms are not allowed in this position. They are only
  // allowed in function call, if the function parameter is annotated with
  // __Atom
  echo $f#Name();
  echo "\n";

  // wrong, we must pass an atom to `with_atom`
  $f = ($x ==> with_atom($x));
  // same
  $f = ((HH\MemberOf<E, string> $x) ==> with_atom($x));
  // more stuble, but wrong. The `__Atom` automatically transform the input
  // atom into a constant, so the $x with pass to with_atom is wrong too
  $f = ((<<__Atom>>HH\MemberOf<E, string> $x) ==> with_atom($x));
  // bottom line, lambdas should use `HH\Label`, not __Atom
}

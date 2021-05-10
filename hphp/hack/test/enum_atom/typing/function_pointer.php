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
  $f = without_atom<>;
  expect_string($f(E::Name));

  echo $f(E::Name);
  echo "\n";


  $f = with_atom<>;
  expect_string($f#Name());

  echo $f#Name();
  echo "\n";
}

function bad(): void {
  $f = without_atom<>;
  expect_int($f(E::Name)); // int vs string

  // Typing[4396] Atoms are not allowed in this position. They are only
  // allowed in function call, if the function parameter is annotated with
  // __Atom
  echo $f#Name();
  echo "\n";


  $f = with_atom<>;
  expect_int($f#Name()); // int vs string


  // Typing[4397] An atom is required here, not a class constant projection
  echo $f(E::Name);
  echo "\n";
}

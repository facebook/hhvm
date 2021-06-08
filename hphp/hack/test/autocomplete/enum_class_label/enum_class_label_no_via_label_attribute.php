<?hh

enum class EE : int {
  int Foo = 1;
  int Bar = 2;
}

// Without the Atom attribute, this shouldn't give any results.
function get_no_atom<T>(HH\MemberOf<EE, T> $z) : T {
  return $z;
}

function test(): void {
  get_no_atom#AUTO332
}

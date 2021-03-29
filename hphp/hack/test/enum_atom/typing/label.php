<?hh
<<file:__EnableUnstableFeatures('enum_atom')>>

interface I {
  public function kind() : string;
}

class Box<T> implements I {
  public function __construct(public T $x, private string $kind)[] {}
  public function kind(): string { return $this->kind; }
}


enum class E :  I {
  Box<string> Name = new Box('zuck', 'NAME');
  Box<int> Age = new Box(42, 'AGE');
}

function expect_Member<T>(HH\MemberOf<E, Box<T>> $member) : void {
  $kind = $member->kind();
}

function expect_string(string $name) : void {
}

function E_value_of_atom<T>(HH\Label<E, Box<T>> $atom): HH\MemberOf<E, Box<T>> {
  return E::valueOf($atom);
}

function E_name_of_atom<T>(HH\Label<E, Box<T>> $atom): string {
  return E::nameOf($atom);
}

function E_value_of<T>(HH\Label<E, Box<T>> $atom): HH\MemberOf<E, Box<T>> {
  return E::valueOf($atom);
}

function E_name_of<T>(HH\Label<E, Box<T>> $atom): string {
  return E::nameOf($atom);
}

function E_test<T>(HH\Label<E, Box<T>> $atom) : void {
  echo E_name_of_atom($atom);
  echo "\n";
  echo E_value_of_atom($atom)->x;
  echo "\n";
  echo E_name_of($atom);
  echo "\n";
  echo E_value_of($atom)->x;
  echo "\n";
}

enum class F :  I extends E {
  Box<string> Foo = new Box('meh', 'FOO');
}

<<__EntryPoint>>
function testit() : void {
  expect_Member(E::Name);
  expect_string(E::Age->kind());
  echo "Testing labels\n";
  echo "[E] Name = ";
  echo E::nameOf#Name();
  echo "\n";

  echo "[E] AGE = ";
  echo E::valueOf#Age()->kind();
  echo "\n";

  echo "[E] 42 = ";
  echo E::valueOf#Age()->x;
  echo "\n";

  echo "[E] 42 = ";
  echo E_value_of_atom#Age()->x;
  echo "\n";

  echo "[E] Age = ";
  echo E_name_of_atom#Age();
  echo "\n";

  echo "Testing indirect calls\n";
  E_test#Name();

  echo "[F] NAME = ";
  echo F::valueOf#Name()->kind();
  echo "\n";

  echo "[F] FOO = ";
  echo F::valueOf#Foo()->kind();
  echo "\n";

  echo "[F] Foo = ";
  echo F::nameOf#Foo();
  echo "\n";
}

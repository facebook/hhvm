<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

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

function E_name_of_label<T>(HH\EnumClass\Label<E, Box<T>> $label): string {
  return E::nameOf($label);
}

function E_value_of_member<T>(HH\EnumClass\Label<E, Box<T>> $label): HH\MemberOf<E, Box<T>> {
  return E::valueOf($label);
}

function E_test<T as arraykey>(HH\EnumClass\Label<E, Box<T>> $atom) : void {
  echo E_name_of_label($atom);
  echo "\n";
  echo E_value_of_member($atom)->x;
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
  echo E_value_of_member#Age()->x;
  echo "\n";

  echo "[E] Age = ";
  echo E_name_of_label#Age();
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

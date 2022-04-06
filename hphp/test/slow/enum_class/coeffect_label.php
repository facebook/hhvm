<?hh

interface I {
  public function kind()[]: string;
}

class Box<T> implements I {
  public function __construct(public T $x, private string $kind)[] {}
  public function kind()[]: string { return $this->kind; }
}

enum class E : I {
  Box<string> Name = new Box('zuck', 'NAME');
  Box<int> Age = new Box(42, 'AGE');
}

function expect_age_member<T>(HH\EnumClass\Label<E, Box<T>> $member)[write_props]: bool {
  return E#Age === $member;
}

function fun_with_write_props()[write_props]: bool {
  return expect_age_member(#Age);
}

<<__EntryPoint>>
function main(): void {
  $result = fun_with_write_props();
  var_dump($result);
}

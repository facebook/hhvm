<?hh

class MyClass {}

case type CT<T> =
| num where T super arraykey
| bool where T = bool
| string where T as num
| MyClass where T super int;

function exact_satisfies_super(float $x): CT<arraykey> {
  return $x;
}

function super_satisfies_super(float $x): CT<?arraykey> {
  return $x;
}

function sub_fails_super(float $x): CT<int> {
  return $x;
}

function other_fails_super(float $x): CT<null> {
  return $x;
}

////

function exact_satisfies_eq(bool $x): CT<bool> {
  return $x;
}

function super_fails_eq(bool $x): CT<?bool> {
  return $x;
}

function sub_fails_eq(bool $x): CT<nothing> {
  return $x;
}

function other_fails_eq(bool $x): CT<null> {
  return $x;
}

////

function exact_satisfies_as(string $x): CT<num> {
  return $x;
}

function super_fails_as(string $x): CT<?num> {
  return $x;
}

function sub_satisfies_as(string $x): CT<int> {
  return $x;
}

function other_fails_as(string $x): CT<null> {
  return $x;
}

////

function takes_ct_num(CT<num> $x): void {}

function test_multiple_variants(bool $flag): void {
  takes_ct_num("string");
  takes_ct_num(new MyClass());
  $val = $flag ? "string" : new MyClass();
  takes_ct_num($val);
}

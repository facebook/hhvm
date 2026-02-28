<?hh

enum E: int {
  A = 42;
  B = 43;
}

function main(): void {
  hh_expect_equivalent<dict<string,E>>(E::getValues());
  hh_expect_equivalent<dict<E,string>>(E::getNames());
  hh_expect_equivalent<bool>(E::isValid(42));
  hh_expect_equivalent<?E>(E::coerce(42));
  hh_expect_equivalent<E>(E::assert(42));
  hh_expect_equivalent<Container<E>>(E::assertAll(vec[42]));
}

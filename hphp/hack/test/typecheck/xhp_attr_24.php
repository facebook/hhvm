<?hh

class :foo extends SomePHPClass {
  attribute int bar;
}

function main(): void {
  $x = <foo bar="abc" />; // type error
}

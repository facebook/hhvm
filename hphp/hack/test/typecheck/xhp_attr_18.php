<?hh

class :foo extends SomePHPClass {
  attribute int bar;
}

function main(): void {
  $x = <foo undefined="baz" />; // no errors
  $x = <foo bar={123} />; // no errors
}

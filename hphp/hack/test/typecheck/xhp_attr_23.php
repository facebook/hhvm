<?hh // strict

class :foo {}

function main(): void {
  $x = <foo bar="baz" />; // undefined attribute
}

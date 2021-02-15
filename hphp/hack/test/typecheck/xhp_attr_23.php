<?hh // strict

class :foo extends XHPTest {}

function main(): void {
  $x = <foo bar="baz" />; // undefined attribute
}

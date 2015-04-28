<?hh // strict

class :foo {}

function main(): void {
  $x = <foo data-bar="baz" aria-herp="derp" />; // no errors
}

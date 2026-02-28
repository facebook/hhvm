<?hh

class :foo extends XHPTest {}

function main(): void {
  $x = <foo data-bar="baz" aria-herp="derp" />; // no errors
}

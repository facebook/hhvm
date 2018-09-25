<?hh // strict

class NullObject extends SimpleXMLElement {}

function test(NullObject $x): void {
  if ($x) {
  }
}

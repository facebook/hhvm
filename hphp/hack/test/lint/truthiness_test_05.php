<?hh // strict

function test(SimpleXMLElement $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}

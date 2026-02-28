<?hh

class NullObject extends SimpleXMLElement {}

function test(NullObject $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}

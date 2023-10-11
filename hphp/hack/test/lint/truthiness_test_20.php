<?hh

class C implements StringishObject{
  public function __toString(): string { return ''; }
}

function test(C $x): void {
  if ($x) {
    do_something();
  }
}

function do_something(): void {}

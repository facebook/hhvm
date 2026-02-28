<?hh

class :something extends :xhp {
  attribute Foo x @required;

  public function render(): void {
    takes_string($this->:x);
  }
}

function takes_string(string $s): void {}

class Foo extends Enum {
  const type TInner = int;
}

class :xhp extends XHPTest {}

class Enum {
  abstract const type TInner;
}

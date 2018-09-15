<?hh // strict

class :xhp implements XHPChild {}
class :foo extends :xhp {}
class :bar extends :xhp {}

class A {
  public function __toString(): string {
    return 'A';
  }
}

function test(): void {
  <foo />;
  <foo></foo>;
  <foo>
    <bar />
  </foo>;
  <foo>
    {null}
  </foo>;
  <foo>
    {3}
  </foo>;
  <foo>test!</foo>;
  <foo>
    {new A()}
  </foo>;
  <bar>
    <foo>{array(vec[<bar />])}</foo>
  </bar>;
}

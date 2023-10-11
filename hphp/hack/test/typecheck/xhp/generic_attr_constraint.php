<?hh

class :my-xhp<T as num> extends XHPTest {
  attribute T foo @required;
}

function test(): void {
  <my-xhp foo="1" />;
}

<?hh // strict

class :my-xhp<T> extends XHPTest {
  attribute T foo @required;
}

function test(int $x): :my-xhp<string> {
  return <my-xhp foo={$x} />;
}

<?hh

class :my-xhp<T> extends XHPTest {
  attribute Vector<T> foo @required;
}

function test(): int {
  $x = <my-xhp foo={Vector { "1" }} />;

  return $x->:foo[0];
}

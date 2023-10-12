<?hh // strict

class :my-xhp<T> extends XHPTest {
  attribute Vector<T> foo @required;
}

function test<T>(T $x): :my-xhp<T> {
  return <my-xhp foo={Vector { $x }} />;
}

function test2(): Vector<arraykey> {
  $xhp = test(10);
  return $xhp->:foo;
}

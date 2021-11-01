<?hh

class :xhp extends XHPTest {
  attribute int foo = 0;
}

function test(:xhp $obj): void {
  // Using ->: syntax for writing is not supported for now
  $obj->:foo = 123;
}

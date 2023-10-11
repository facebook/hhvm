<?hh

class :my-xhp extends XHPTest {
  attribute this me;
}

function test(:my-xhp $y): :my-xhp {
  return <my-xhp me={$y} />;
}

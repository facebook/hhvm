<?hh // strict

class :my-xhp {
  attribute this me;
}

function test(:my-xhp $y): :my-xhp {
  return <my-xhp me={$y} />;
}

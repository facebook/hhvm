<?hh // strict

class :foo {
  attribute int name, int age;
}

class :bar extends XHPTest {
  attribute string name;
}

function test(:foo $x): :bar {
  // Name is not compatible across the spread
  return <bar {...$x} />;
}

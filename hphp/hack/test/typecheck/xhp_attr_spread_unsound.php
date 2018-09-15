<?hh // strict

class :foo {
  attribute string name;
}

class :subfoo extends :foo {
  attribute string age;
}

class :bar {
  attribute int age;
}

function get_foo(): :foo {
  return <subfoo />;
}

function test(): void {
  $foo = get_foo();
  // No errors, even though we are copying string :age into int :age since we
  // have :subfoo at runtime
  <bar {...$foo} />;
}

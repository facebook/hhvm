<?hh // strict
/**
 * Tests whether or not XHP attribute types are correctly localized when we try
 * to spread them.
 */
class :foo {
  attribute this my-foo;
}

class :bar {
  attribute :foo my-foo, string name;
}

class :baz {
  attribute this my-foo;
}

function test(): void {
  $x = <foo my-foo={<foo />} />;
  // This should work, my-foo is a :foo in $x, which is compatible
  <bar {...$x} />;
  // And this should fail, because it's not a :baz
  <baz {...$x} />;
}

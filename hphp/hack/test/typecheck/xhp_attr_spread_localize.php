<?hh
/**
 * Tests whether or not XHP attribute types are correctly localized when we try
 * to spread them.
 */
class :foo extends XHPTest {
  attribute this my-foo;
}

class :bar extends XHPTest {
  attribute :foo my-foo, string name;
}

class :baz extends XHPTest {
  attribute this my-foo;
}

function test(): void {
  $x = <foo my-foo={<foo />} />;
  // This should work, my-foo is a :foo in $x, which is compatible
  <bar {...$x} />;
  // And this should fail, because it's not a :baz
  <baz {...$x} />;
}

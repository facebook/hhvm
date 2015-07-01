<?hh

/*
 * Baz::f is inconsistent with Foo:f, but we don't detect this error because
 * we only check the direct parent of a class for consistency. We should
 * solve this eventually by forcing all child class methods / members to have
 * type hints if the corresponding method / member in the parent is hinted.
 * That way the consistency check will be transitive.
 */

class Foo {
  public function f(string $x) {}
}

class Bar {
  public function f($x) {}
}

class Baz {
  public function f(int $x) {}
}

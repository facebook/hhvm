<?hh

class Foo {
  public function __construct(
    public string $a = 'a',
  ) {}
}

function f() {
  $x = new Foo();

  // This should raise an error because we are incrementing a string.
  // There used to be a bug where postfix operators were dropped when
  // they appeared after higher-precedence operators like '->'.
  $x->a++;
}

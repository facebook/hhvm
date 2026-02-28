<?hh


class Foo {
  <<__Memoize>>
  public readonly function memoized_method(): readonly Foo {
    echo "memoized_method\n";
    return $this;
  }
}

<<__Memoize>>
function memoized_return_readonly(): readonly Foo {
  echo "memoized_function";
  return new Foo();
}

<<__EntryPoint>>
function test(): void {
  $x = new Foo();

  readonly memoized_return_readonly();
  $x->memoized_method();
}

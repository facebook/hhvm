<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Foo {
  public int $prop = 2;
}
function foo(readonly Foo $x): readonly Foo {
  $y = HH\Readonly\as_mut(readonly $x->prop);
  var_dump($y);
  return $x;
}

class Test {
  public function __construct(
    public readonly Foo $prop
  ) {}

  public readonly function get(): readonly Foo {
    return readonly $this->prop;
  }
}


class ReadonlyBox<T> {
  public function __construct(
    private readonly T $contents
  ) {}

  public readonly function get(): readonly T {
    return readonly $this->contents;
  }

  public function set(readonly T $x): void {
    $this->contents = $x;
  }
}

<<__EntryPoint>>
function main(): void {
  $y = new Test(new Foo());
  $a = readonly $y->prop;
  $y = new ReadonlyBox($a);
  $z = readonly $y->get();
  readonly foo($z);
  echo "Done\n";
}

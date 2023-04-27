<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public ?int $x;
  public function reset(): int {
    return 1;
  }
}

function lift<T>(T $_): ExprTree<ExampleDsl, ExampleDsl::TAst, T> {
  throw new Exception();
}

function test(): void {
  $x = new Foo();

  if ($x->x !== null) {
    $_ = ExampleDsl`() ==> {
      ${lift($x->reset())};
      return;
    }`;

    // We should think that $x->x could be null
    $x->x + 1;
  }
}

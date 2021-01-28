<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class Foo {
  public ?int $x;
  public function reset(): int {
    return 1;
  }
}

function lift<T>(T $_): ExprTree<Code, Code::TAst, T> {
  throw new Exception();
}

function test(): void {
  $x = new Foo();

  if ($x->x !== null) {
    $_ = Code`() ==> {
      ${lift($x->reset())};
      // We should think that $x->x could be null
      ${lift($x->x + 1)};
      return;
    }`;
  }
}

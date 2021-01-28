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

// This technically shouldn't throw an error.
// It currently is due to typechecking the current desugaring
// So, for the moment, allow this error to be thrown and fix the desugaring
function test(): void {
  $x = new Foo();

  if ($x->x !== null) {
    $_ = Code`() ==> {
      // We know that $x->x is not null
      ${lift($x->x + 1)};
      return;
    }`;
  }
}

<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Placeholder definition so we don't get naming/typing errors.
class Code {
  const type TAst = mixed;
  // Simple literals.
  public function nullLiteral(): this::TAst {
    throw new Exception();
  }
}

function foo(): void {
  $n = Code`null`;
}

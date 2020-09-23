<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class Code {
  const type TAst = mixed;
  // Simple literals.
  public function intLiteral(int $_): this::TAst {
    throw new Exception();
  }

  // Operators
  public function plus(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }

  // TODO: it would be better to discard unsupported syntax nodes during lowering.
  public function unsupportedSyntax(string $msg): this::TAst {
    throw new Exception($msg);
  }
}

function test(): void {
  Code`__splice__(1 + __splice__(4))`;
}

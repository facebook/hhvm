<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

final class Code {
  const type TAst = mixed;

  // TODO: it would be better to discard unsupported syntax nodes during lowering.
  public function unsupportedSyntax(string $msg): this::TAst {
    throw new Exception($msg);
  }
}

function test(): void {
  Code`__splice__(1 << 4)`;
}

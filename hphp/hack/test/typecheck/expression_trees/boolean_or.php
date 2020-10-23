<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $n = Code`true || false`;
}

//// BEGIN DEFS
// Placeholder definition so we don't get naming/typing errors.
final class Code {
  const type TAst = mixed;
  // Simple literals.
  public function intLiteral(int $_): this::TAst {
    throw new Exception();
  }
  public function boolLiteral(bool $_): this::TAst {
    throw new Exception();
  }
  public function stringLiteral(string $_): this::TAst {
    throw new Exception();
  }
  public function nullLiteral(): this::TAst {
    throw new Exception();
  }
  public function localVar(string $_): this::TAst {
    throw new Exception();
  }
  public function lambdaLiteral(
    vec<string> $_args,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }

  // Operators
  public function plus(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function ampamp(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function barbar(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function exclamationMark(this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function call(string $_fnName, vec<this::TAst> $_args): this::TAst {
    throw new Exception();
  }

  public function assign(this::TAst $_, this::TAst $_): this::TAst {
    throw new Exception();
  }

  // Statements.
  public function ifStatement(
    this::TAst $_cond,
    vec<this::TAst> $_then_body,
    vec<this::TAst> $_else_body,
  ): this::TAst {
    throw new Exception();
  }
  public function whileStatement(
    this::TAst $_cond,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }
  public function returnStatement(?this::TAst $_): this::TAst {
    throw new Exception();
  }
  public function forStatement(
    vec<this::TAst> $_,
    this::TAst $_,
    vec<this::TAst> $_,
    vec<this::TAst> $_,
  ): this::TAst {
    throw new Exception();
  }
  public function breakStatement(): this::TAst {
    throw new Exception();
  }
  public function continueStatement(): this::TAst {
    throw new Exception();
  }

  // Splice
  public function splice<T>(ExprTree<this, this::TAst, T> $_): this::TAst {
    throw new Exception();
  }

  // TODO: Discard unsupported syntax nodes while lowering
  public function unsupportedSyntax(string $msg): this::TAst {
    throw new Exception($msg);
  }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private (function(TVisitor): TResult) $x,
    private (function(): TInfer) $err,
  ) {}
}
//// END DEFS

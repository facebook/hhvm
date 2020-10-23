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
      __splice__(lift($x->x + 1));
      return;
    }`;
  }
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

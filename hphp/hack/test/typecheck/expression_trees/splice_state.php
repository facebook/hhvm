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
  public function intLiteral(?ExprPos $_, int $_): this::TAst {
    throw new Exception();
  }
  public function boolLiteral(?ExprPos $_, bool $_): this::TAst {
    throw new Exception();
  }
  public function stringLiteral(?ExprPos $_, string $_): this::TAst {
    throw new Exception();
  }
  public function nullLiteral(?ExprPos $_): this::TAst {
    throw new Exception();
  }
  public function localVar(?ExprPos $_, string $_): this::TAst {
    throw new Exception();
  }
  public function lambdaLiteral(
    ?ExprPos $_,
    vec<string> $_args,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }

  // Operators
  public function plus(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function ampamp(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function barbar(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function exclamationMark(
    ?ExprPos $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function call(
    ?ExprPos $_,
    string $_fnName,
    vec<this::TAst> $_args,
  ): this::TAst {
    throw new Exception();
  }

  public function assign(
    ?ExprPos $_,
    this::TAst $_,
    this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }

  // Statements.
  public function ifStatement(
    ?ExprPos $_,
    this::TAst $_cond,
    vec<this::TAst> $_then_body,
    vec<this::TAst> $_else_body,
  ): this::TAst {
    throw new Exception();
  }
  public function whileStatement(
    ?ExprPos $_,
    this::TAst $_cond,
    vec<this::TAst> $_body,
  ): this::TAst {
    throw new Exception();
  }
  public function returnStatement(
    ?ExprPos $_,
    ?this::TAst $_,
  ): this::TAst {
    throw new Exception();
  }
  public function forStatement(
    ?ExprPos $_,
    vec<this::TAst> $_,
    this::TAst $_,
    vec<this::TAst> $_,
    vec<this::TAst> $_,
  ): this::TAst {
    throw new Exception();
  }
  public function breakStatement(?ExprPos $_): this::TAst {
    throw new Exception();
  }
  public function continueStatement(?ExprPos $_,): this::TAst {
    throw new Exception();
  }

  // Splice
  public function splice<T>(
    ?ExprPos $_,
    ExprTree<this, this::TAst, T> $_,
  ): this::TAst {
    throw new Exception();
  }

  // TODO: Discard unsupported syntax nodes while lowering
  public function unsupportedSyntax(string $msg): this::TAst {
    throw new Exception($msg);
  }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private ?ExprPos $pos,
    private (function(TVisitor): TResult) $x,
    private (function(): TInfer) $err,
  ) {}
}

final class ExprPos {
  public function __construct(
    private int $begin_line,
    private int $begin_col,
    private int $end_line,
    private int $end_col,
  ) {}
}
//// END DEFS

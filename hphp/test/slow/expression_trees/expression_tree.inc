<?hh

type ExprTreeInferredType<T> = ?(function(): T);

function concat_arg_list(vec<string> $args): string {
  return implode(",", $args);
}

function concat_block(vec<string> $block): string {
  return implode("\n", $block);
}

class Code {
  const type TAst = string;

  public static function makeTree<TVisitor as Code, TInfer>(
    ?ExprPos $pos,
    dict<string, mixed> $spliced_values,
    (function(TVisitor): Code::TAst) $ast,
    ExprTreeInferredType<TInfer> $null,
  ): ExprTree<TVisitor, Code::TAst, TInfer> {
    return new ExprTree($pos, $spliced_values, $ast, $null);
  }

  // Lifting literals.
  public static function liftInt(
    int $i,
  ): ExprTree<Code, Code::TAst, ExampleInt> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> (string)$i,
      () ==> {
        throw new Exception();
      },
    );
  }

  public static function liftFloat(
    float $f,
  ): ExprTree<Code, Code::TAst, ExampleFloat> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> (string)$f,
      () ==> {
        throw new Exception();
      },
    );
  }

  public static function liftBool(
    bool $b,
  ): ExprTree<Code, Code::TAst, ExampleBool> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> $b ? "true" : "false",
      () ==> {
        throw new Exception();
      },
    );
  }

  public static function liftString(
    string $s,
  ): ExprTree<Code, Code::TAst, ExampleString> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> "\"$s\"",
      () ==> {
        throw new Exception();
      },
    );
  }

  public static function liftNull(): ExprTree<Code, Code::TAst, null> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> "null",
      () ==> {
        throw new Exception();
      },
    );
  }

  public static function liftVoid(): ExprTree<Code, Code::TAst, ExampleVoid> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> "void",
      () ==> {
        throw new Exception();
      },
    );
  }

  // Symbols
  public static function liftSymbol<T>(
    (function(ExampleContext): Awaitable<ExprTree<Code, Code::TAst, T>>) $_,
  ): ExprTree<Code, Code::TAst, T> {
    return new ExprTree(
      null,
      dict[],
      (Code $_) ==> "function_ptr",
      () ==> {
        throw new Exception();
      },
    );
  }

  // Expressions
  public function visitLocal(?ExprPos $_, string $local): Code::TAst {
    return $local;
  }

  public function visitLambda(
    ?ExprPos $_,
    vec<string> $args,
    vec<Code::TAst> $body,
  ): Code::TAst {
    return "(".concat_arg_list($args).")"." ==> {\n".concat_block($body)."}";
  }

  // Operators
  public function visitMethCall(
    ?ExprPos $_,
    Code::TAst $callee,
    string $op,
    vec<Code::TAst> $args,
  ): Code::TAst {
    return $callee."->".$op."(".concat_arg_list($args).")";
  }

  public function visitCall<T>(
    ?ExprPos $_,
    Code::TAst $callee,
    vec<Code::TAst> $args,
  ): Code::TAst {
    return $callee."(".concat_arg_list($args).")";
  }

  public function visitAssign(
    ?ExprPos $_,
    Code::TAst $local,
    Code::TAst $value,
  ): Code::TAst {
    return $local." = ".$value.";";
  }

  public function visitTernary(
    ?ExprPos $_,
    Code::TAst $condition,
    ?Code::TAst $truthy,
    Code::TAst $falsy,
  ): Code::TAst {
    return $condition." ? ".$truthy." : ".$falsy;
  }

  public function visitIf(
    ?ExprPos $_,
    Code::TAst $cond,
    vec<Code::TAst> $then_body,
    vec<Code::TAst> $else_body,
  ): Code::TAst {
    return "if (".
      $cond.
      ") {\n".
      concat_block($then_body).
      "} else {\n".
      concat_block($else_body).
      "}";
  }

  public function visitWhile(
    ?ExprPos $_,
    Code::TAst $cond,
    vec<Code::TAst> $body,
  ): Code::TAst {
    return "while (".$cond.") {\n".concat_block($body)."}";
  }

  public function visitReturn(?ExprPos $_, Code::TAst $return_val): Code::TAst {
    return "return ".$return_val.";";
  }

  public function visitFor(
    ?ExprPos $_,
    vec<Code::TAst> $initializers,
    ?Code::TAst $cond,
    vec<Code::TAst> $increments,
    vec<Code::TAst> $body,
  ): Code::TAst {
    return "for (".
      concat_arg_list($initializers).
      ";".
      ($cond ?? "").
      ";".
      concat_arg_list($increments).
      ") {\n".
      concat_block($body).
      "}";
  }

  public function visitBreak(?ExprPos $_): Code::TAst {
    return "break;";
  }

  public function visitContinue(?ExprPos $_): Code::TAst {
    return "continue;";
  }

  // Splice
  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    Spliceable<Code, Code::TAst, T> $splice_val,
  ): Code::TAst {
    return "\${".($splice_val->visit($this))."}";
  }
}

interface Spliceable<TVisitor, TResult, +TInfer> {
  public function visit(TVisitor $v): TResult;
}

final class ExprTree<TVisitor, TResult, +TInfer>
  implements Spliceable<TVisitor, TResult, TInfer> {
  public function __construct(
    private ?ExprPos $pos,
    private dict<string, mixed> $spliced_values,
    private (function(TVisitor): TResult) $ast,
    private ExprTreeInferredType<TInfer> $err,
  ) {}

  public function visit(TVisitor $v): TResult {
    return ($this->ast)($v);
  }

  public function getExprPos(): ?ExprPos {
    return $this->pos;
  }

  public function getSplices(): dict<string, mixed> {
    return $this->spliced_values;
  }
}

final class ExprPos {
  public function __construct(
    public string $filepath,
    public int $begin_line,
    public int $begin_col,
    public int $end_line,
    public int $end_col,
  ) {}
}

abstract class ExampleMixed {
  public abstract function __tripleEquals(ExampleMixed $_): ExampleBool;
  public abstract function __notTripleEquals(ExampleMixed $_): ExampleBool;
}
abstract class ExampleInt extends ExampleMixed {
  public abstract function __plus(ExampleInt $_): ExampleInt;
  public abstract function __minus(ExampleInt $_): ExampleInt;
  public abstract function __star(ExampleInt $_): ExampleInt;
  public abstract function __slash(ExampleInt $_): ExampleInt;
  public abstract function __percent(ExampleInt $_): ExampleInt;
  public abstract function __negate(): ExampleInt;

  public abstract function __lessThan(ExampleInt $_): ExampleBool;
  public abstract function __lessThanEqual(ExampleInt $_): ExampleBool;
  public abstract function __greaterThan(ExampleInt $_): ExampleBool;
  public abstract function __greaterThanEqual(ExampleInt $_): ExampleBool;

  public abstract function __amp(ExampleInt $_): ExampleInt;
  public abstract function __bar(ExampleInt $_): ExampleInt;
  public abstract function __caret(ExampleInt $_): ExampleInt;
  public abstract function __lessThanLessThan(ExampleInt $_): ExampleInt;
  public abstract function __greaterThanGreaterThan(ExampleInt $_): ExampleInt;
  public abstract function __tilde(): ExampleInt;
}

abstract class ExampleBool extends ExampleMixed {
  public abstract function __ampamp(ExampleBool $_): ExampleBool;
  public abstract function __barbar(ExampleBool $_): ExampleBool;
  public abstract function __bool(): bool;
  public abstract function __exclamationMark(): ExampleBool;
}

abstract class ExampleString extends ExampleMixed {
  public abstract function __dot(ExampleString $_): ExampleString;
}

abstract class ExampleFloat extends ExampleMixed {}

abstract class ExampleVoid extends ExampleMixed {}

final class ExampleContext {}

function print_et<TInfer>(ExprTree<Code, Code::TAst, TInfer> $et): void {
  $visitor = new Code();
  $string = $et->visit($visitor);
  echo($string."\n");
}

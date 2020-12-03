<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(
  ExampleContext $_,
): ExprTree<Code, Code::TAst, (function(string): int)> {
  throw new Exception();
}

<<__EntryPoint>>
function test(): void {
  $et = MyVisitor`1 + foo("bar")`;

  $visitor = new MyVisitor();

  $res = $et->construct($visitor);

  echo($res);
}

final class MyVisitor {
  public static function intLiteral(
    int $i,
  ): ExprTree<MyVisitor, mixed, int> {
    return new ExprTree(
      null,
      null,
      dict[],
      (MyVisitor $_) ==> (string)$i,
      () ==> { throw new Exception(); },
    );
  }

  public static function stringLiteral(
    string $s
  ): ExprTree<MyVisitor, mixed, string> {
    return new ExprTree(
      null,
      null,
      dict[],
      (MyVisitor $_) ==> "\"$s\"",
      () ==> { throw new Exception(); },
    );
  }

  public function methCall(
    ExprPos $_,
    mixed $lhs,
    string $meth_name,
    vec<mixed> $rhs,
  ): string {
    $rhs = $rhs[0];
    if ($lhs is string && $rhs is string) {
      return "$lhs $meth_name $rhs";
    }
    return "lhs $meth_name rhs";
  }

  public function call<T>(
    ?ExprPos $_,
    this::TAst $callee,
    vec<this::TAst> $args,
  ): this::TAst {
    $call = "$callee(";
    foreach ($args as $arg) {
      if ($arg is string) {
        $call = $call . "$arg, ";
      } else {
        $call = $call . "arg, ";
      }
    }
    return $call . ")" ;
  }

  public function splice(
    ExprPos $_,
    string $_key,
    ExprTree<MyVisitor, mixed, mixed> $et,
  ): mixed {
    return $et->construct($this);
  }

  public static function symbol<T>(
    string $name,
    (function(ExampleContext): ExprTree<this, this::TAst, T>) $_,
  ): ExprTree<this, this::TAst, T> {
    return new ExprTree(
      null,
      null,
      dict[],
      (MyVisitor $_) ==> { return $name; },
      () ==> { throw new Exception(); },
    );
  }
}

final class ExprTree<TVisitor, TResult, TInfer>{
  public function __construct(
    private ?ExprPos $pos,
    private ?string $filepath,
    private dict<string, mixed> $cached_dict,
    private (function(TVisitor): TResult) $ast,
    private (function(): TInfer) $err,
  ) {}

  public function construct(TVisitor $v): TResult {
    return ($this->ast)($v);
  }
}

final class ExprPos {
  public function __construct(
    private int $begin_line,
    private int $begin_col,
    private int $end_line,
    private int $end_col,
  ) {}
}

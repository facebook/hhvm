<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

class MyVisitorContext {}

function foo(
  MyVisitorContext $_,
): ExprTree<MyVisitor, string, (function(string): MyVisitorInt)> {
  throw new Exception();
}

<<__EntryPoint>>
function test(): void {
  $et = MyVisitor`1 + foo("bar")`;

  $visitor = new MyVisitor();

  $res = $et->visit($visitor);

  echo($res);
}

final class MyVisitor {
  public static function makeTree<TInfer>(
    ?ExprPos $pos,
    dict<string, mixed> $cached_dict,
    (function(MyVisitor): string) $ast,
    ?(function(): TInfer) $err,
  ): ExprTree<MyVisitor, string, TInfer> {
    return new ExprTree($pos, $cached_dict, $ast, $err);
  }

  public static function liftInt(
    int $i,
  ): ExprTree<MyVisitor, string, MyVisitorInt> {
    return new ExprTree(
      null,
      dict[],
      (MyVisitor $_) ==> (string)$i,
      () ==> { throw new Exception(); },
    );
  }

  public static function liftString(
    string $s
  ): ExprTree<MyVisitor, string, string> {
    return new ExprTree(
      null,
      dict[],
      (MyVisitor $_) ==> "\"$s\"",
      () ==> { throw new Exception(); },
    );
  }

  public function methCall(
    ExprPos $_,
    string $lhs,
    string $meth_name,
    vec<string> $rhs,
  ): string {
    $rhs = $rhs[0];
    if ($lhs is string && $rhs is string) {
      return "$lhs $meth_name $rhs";
    }
    return "lhs $meth_name rhs";
  }

  public function call<T>(
    ?ExprPos $_,
    string $callee,
    vec<string> $args,
  ): string {
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
    ExprTree<MyVisitor, string, mixed> $et,
  ): string {
    return $et->visit($this);
  }

  public static function liftSymbol<T>(
    (function(MyVisitorContext): ExprTree<MyVisitor, string, T>) $_,
  ): ExprTree<MyVisitor, string, T> {
    return new ExprTree(
      null,
      dict[],
      (MyVisitor $_) ==> { return 'function_ptr'; },
      () ==> { throw new Exception(); },
    );
  }
}

interface Spliceable<TVisitor, TResult, +TInfer> {
  public function visit(TVisitor $v): TResult;
}


final class ExprTree<TVisitor, TResult, +TInfer> implements Spliceable<TVisitor, TResult, TInfer> {
  public function __construct(
    private ?ExprPos $pos,
    private dict<string, mixed> $cached_dict,
    private (function(TVisitor): TResult) $ast,
    private ?(function(): TInfer) $err,
  ) {}

  public function visit(TVisitor $v): TResult {
    return ($this->ast)($v);
  }
}

final class ExprPos {
  public function __construct(
    private string $path,
    private int $begin_line,
    private int $begin_col,
    private int $end_line,
    private int $end_col,
  ) {}
}

abstract class MyVisitorInt {
  abstract public function __plus(MyVisitorInt $_): MyVisitorInt;
}

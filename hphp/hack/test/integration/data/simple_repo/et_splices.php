<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $lift_one = $_ ==> ExampleDsl`1`;
  $one_splice = ExampleDsl`${$lift_one(1)} + 1`;
}

// smaller subset of hphp/hack/test/expr_tree.php

interface Spliceable<TVisitor, +TResult, +TInfer> {
  public function visit(TVisitor $v): TResult;
}

type ExampleDslExpression<T> = Spliceable<ExampleDsl, ExampleDsl::TAst, T>;

interface ExampleInt {
  public function __plus(ExampleInt $_): ExampleInt;
}

type ExprTreeInfo<TInfer> = shape(
  'splices' => dict<string, mixed>,
  'functions' => vec<mixed>,
  'static_methods' => vec<mixed>,
  // The virtualised expression is placed here, to cause the type checker to instantiate
  // TInfer to the virtualised type.
  ?'type' => (function(): TInfer),
);

final class ExprTree<TVisitor, TResult, +TInfer>
  implements Spliceable<TVisitor, TResult, TInfer> {
  public function __construct(
    private ?ExprPos $pos,
    private ExprTreeInfo<TInfer> $metadata,
    private (function(TVisitor): TResult) $ast,
  )[] {}

  public function visit(TVisitor $v): TResult {
    return ($this->ast)($v);
  }

  public function getExprPos(): ?ExprPos {
    return $this->pos;
  }

  public function getSplices(): dict<string, mixed> {
    return $this->metadata['splices'];
  }
}

// The DSL itself: used as in ExampleDsl`...`. hackc generates a call to makeTree, which
// should return something that implements Spliceable, here an ExprTree
class ExampleDsl {
  const type TAst = string;

  // The desugared expression tree literal will call this method.
  public static function makeTree<TInfer>(
    ?ExprPos $pos,
    shape(
      'splices' => dict<string, mixed>,
      'functions' => vec<mixed>,
      'static_methods' => vec<mixed>,
      ?'type' => (function(): TInfer),
    ) $metadata,
    (function(ExampleDsl): ExampleDsl::TAst) $ast,
  )[]: ExprTree<ExampleDsl, ExampleDsl::TAst, TInfer> {
    return new ExprTree($pos, $metadata, $ast);
  }

  public static function lift<T>(ExprTree<ExampleDsl, ExampleDsl::TAst, T> $x): ExprTree<ExampleDsl, ExampleDsl::TAst, T> {
  return $x;
  }

  // Virtual types. These do not have to be implemented, as they are only used
  // in the virtualized version of the expression tree, to work out the virtual type
  // of literals during type checking.
  public static function intType()[]: ExampleInt {
    throw new Exception();
  }

  // Desugared nodes. Calls to these are emitted by hackc, following the structure
  // of the expression in the expression tree. Here, they compute a string
  // representation of that expression.
  public function visitInt(?ExprPos $_, int $i)[]: ExampleDsl::TAst {
    return (string)$i;
  }

  // Expressions
  public function visitBinop(
    ?ExprPos $_,
    ExampleDsl::TAst $lhs,
    string $op,
    ExampleDsl::TAst $rhs,
  )[]: ExampleDsl::TAst {
    return "$lhs $op $rhs";
  }

  public function splice<T>(
    ?ExprPos $_,
    string $_key,
    ExampleDslExpression<T> $splice_val,
  ): ExampleDsl::TAst {
    return "\${".($splice_val->visit($this))."}";
  }
}

type ExprPos = shape(...);

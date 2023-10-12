<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Expression<Tresult> {}
abstract class Evaluator<T, Texpression as Expression<T>> {}

class LiteralEvaluator<T, Texpression as ILiteralExpression<T>>
  extends Evaluator<T, Texpression> {}

interface ILiteralExpression<TlitResult> {
  require extends Expression<TlitResult>;
}

<?hh

function get_fcontext(): IContextF {
  throw new Exception();
}

interface IContextP {}

interface IContextF extends IContextP {}

interface IContextG extends IContextP {}

abstract class Taker {
  abstract const type TContext;
  final public static function take(this::TContext $vc): void {
    throw new Exception();
  }
}

final class FTaker extends Taker {
  const type TContext = IContextF;
}

function foo(IContextP $context): void {
  if ($context is IContextG) {
    $context = get_fcontext();
  } else {
    $context = $context as ?IContextF;
  }
  FTaker::take($context);
}

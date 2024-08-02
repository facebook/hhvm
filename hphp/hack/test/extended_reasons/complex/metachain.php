<?hh

async function main<T>(
  ReproMetaChain<T, shape()> $val,
  ReproIMetaChainOperator<shape(), (nothing, string)> $obj,
): Awaitable<void> {
  $foo = await genX(
    // ReproMetaChain::fromValue(shape()) instead of $val
    $val->then($obj),
  );
  list($_, $user_prompt) = $foo;
  //$user_prompt = $foo[1];
  $s = shape(
    'prompt' => $user_prompt,
  );
  $chain = chain($s);
  $chain->thenMPZ(ReproIGCreatorAIMetachain::class);
}

final class ReproIGCreatorAIMetachain implements ReproIGenesisRunnable {
  const type TInput = shape(
    'prompt' => string,
  );
}

<<__NoAutoLikes>>
function chain<T>(T $_): ReproGenesisChain<T> {
  return null as nonnull;
}

final class ReproGenesisChain<T> {

  public function thenMPZ(
    classname<ReproIGenesisRunnable with { type TInput super T; }> $_,
  ): ReproGenesisChain<mixed> {
    return null as nonnull;
  }
}

interface ReproIGenesisRunnable
  extends ReproIMetaChainOperator<shape(), mixed> {

  abstract const type TInput;
}

final class ReproMetaChain<TIn, TOut>
  implements ReproIMetaChainOperator<TIn, TOut> {

  public static function fromValue(TOut $_): ReproMetaChain<TIn, TOut> {
    return null as nonnull;
  }
  public function then<TNextOut>(
    ReproIMetaChainOperator<TOut, TNextOut> $op,
  ): ReproMetaChain<TIn, TNextOut> {
    return null as nonnull;
  }
}

interface ReproIMetaChainOperator<TIn, TOut> {}

async function genX<TIn, TOut>(
  ReproIMetaChainOperator<TIn, TOut> $_,
): Awaitable<TOut> {
  return null as nonnull;
}

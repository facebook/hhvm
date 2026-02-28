<?hh

async function main(
  R $rt,
  SM $sm,
  MC<null,shape()> $foo,
  MC<null, shape('f' => vec<shape()>)> $bar,
): Awaitable<void> {

  $x = $bar->transform((SM::TOutput $o) ==> $o['f']);
  $thunk = $rt->genXThunk($x);
  // Produces ~~vec<_>
  $y = await $thunk;
  $val2 = shape('a' => $y);
  $baz = MC::fromValue($val2);
  $tmp = new RS();
  $baz->then($tmp);
}

final class R {
  public async function genXThunk<TOut>(
    MCO<null, TOut> $f,
  ): Awaitable<TOut> {
    throw new Exception("A");
  }
}

final class MC<TIn, TOut>
  implements MCO<TIn, TOut> {
  public static function fromValue(TOut $val): MC<TIn, TOut> {
    throw new Exception("A");
  }

  public function then<
    TOperator as MCO<TOut, TNextOut>,
    TNextOut,
  >(TOperator $op): MC<TIn, TNextOut> {
    throw new Exception("A");
  }

  public function transform<TNextOut>(
    (function(TOut): TNextOut) $fn,
  ): MC<TIn, TNextOut> {
    throw new Exception("A");
  }
}

interface MCO<TIn, TOut> {}

final class SM
  implements MCO<this::TInput, this::TOutput> {

  const type TInput = shape();

  const type TOutput = shape(
    'f' => vec<RE>,
  );
}

final class RS implements MCO<this::TInput, mixed> {
  const type TInput = shape(
    'a' => vec<RE>,
  );
}

type RE = shape();

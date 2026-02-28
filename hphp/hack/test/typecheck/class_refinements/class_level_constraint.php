<?hh



interface IContextHost {
  abstract const type TCtx;
}

final class UsesReifiedContextHost<THost as IContextHost with { type TCtx = TC }, -TC> {
  private (function(TC): void) $fn;

  public function __construct((function(TC): void) $fn)[] {
    $this->fn = $fn;
  }

  public function do(TC $ctx): void {
    $f = $this->fn;
    $f($ctx);
  }
}

class MyContextHost implements IContextHost {
  const type TCtx = mixed;
}

function test_it_impl(mixed $ctx): void {
  $good = (mixed $_) ==> {};
  $bad = (int $_) ==> {};
  $c = new UsesReifiedContextHost<MyContextHost, MyContextHost::TCtx>($good);
  $c->do($ctx); // NOTE: using $bad instead of $good results in a Hack error:
  // Typing[4110] Invalid argument [1]
}

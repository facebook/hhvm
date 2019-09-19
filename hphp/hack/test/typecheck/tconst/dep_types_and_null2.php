<?hh // strict

class C {}

function expects_C<T as C>(?T $x): void {}

/* HH_FIXME[4336] */
function not_null<T>(?T $x): T {
}

async function test(bool $cond): Awaitable<C> {
  $c = null;
  // force integration
  if ($cond) {
    $c = new C();
  }
  hh_show($c);
  expects_C($c);
  hh_show($c);
  // the null is successfully unwrapped
  $c = not_null($c);
  hh_show($c);
  return $c;
}

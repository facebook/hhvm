<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Blah {
  public async function poly<T>((function(): Awaitable<T>) $_): Awaitable<?T> {
    return null;
  }
}

function refIt(): void {
  $f = meth_caller(Blah::class, 'poly');
  hh_expect<
    HH\FunctionRef<(readonly function<T>(
      Blah,
      (function(): Awaitable<T>),
    ): Awaitable<?T>)>,
  >($f);
}

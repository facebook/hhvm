<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

type My<+T> = T;

class Blah {
  public function poly<T>((function(): My<T>) $_): My<?T> {
    return null;
  }
}

function refIt(): void {
  $f = meth_caller(Blah::class, 'poly');
  hh_expect<
    HH\FunctionRef<(readonly function<T>(Blah, (function(): My<T>)): My<?T>)>,
  >($f);
}

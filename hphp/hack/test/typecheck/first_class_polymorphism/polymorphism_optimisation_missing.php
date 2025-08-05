<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Blah {
  public function poly<T>((function(): Nope<T>) $_): Nope<?T> {
    throw new Exception("not implemented");
  }
}

function refIt(): void {

  $f = meth_caller(Blah::class, 'poly');
  hh_expect<
    HH\FunctionRef<(readonly function<T>(
      Blah,
      (function(): Nope<T>),
    ): Nope<?T>)>,
  >($f);
}

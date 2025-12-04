<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class BoundedTyconst {
  abstract const type Ta super Wibble;

  public function elaborate(this::Ta $_): void {}
}

interface Wibble {
}

function test(): void {
  $fptr = meth_caller(BoundedTyconst::class, 'elaborate');
  hh_expect<
    HH\FunctionRef<(readonly function<Ta1 super Wibble>(
      BoundedTyconst with { type Ta = Ta1 },
      Ta1,
    ): void)>,
  >($fptr);
}

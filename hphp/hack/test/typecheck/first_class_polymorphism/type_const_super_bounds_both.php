<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class BoundedTyconst {
  abstract const type Ta as Wobble super Wibble;

  public function elaborate(this::Ta $_): void {}
}

abstract class Wobble {
}

abstract class Wibble extends Wobble {
}

function test(): void {
  $fptr = meth_caller(BoundedTyconst::class, 'elaborate');
  hh_expect<
    HH\FunctionRef<(readonly function<Ta1 as Wobble super Wibble>(
      BoundedTyconst with { type Ta = Ta1 },
      Ta1,
    ): void)>,
  >($fptr);
}

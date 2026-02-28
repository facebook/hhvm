<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class BoundedTyconst {
  abstract const type Ta as Wobble super Wibble;

  public function elaborate(this::Ta $_, this::Ta::Tb $_): void {}
}

abstract class Wobble {
  abstract const type Tb as Huge super Tiny;
}
abstract class Wibble extends Wobble {
  abstract const type Tb as Bigly super Smol;
}

abstract class Huge {
}
abstract class Bigly extends Huge {
}
abstract class Smol extends Bigly {
}
abstract class Tiny extends Smol {
}

function test(): void {
  $fptr = meth_caller(BoundedTyconst::class, 'elaborate');
  hh_expect<
    HH\FunctionRef<(readonly function<
      Ta1 as Wobble with { type Tb = Tb1 } super Wibble with { type Tb = Tb1 },
      Tb1 as Bigly super Smol,
    >(BoundedTyconst with { type Ta = Ta1 }, Ta1, Tb1): void)>,
  >($fptr);
}

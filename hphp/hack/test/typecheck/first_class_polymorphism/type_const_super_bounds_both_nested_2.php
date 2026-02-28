<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class BoundedTyconst {
  abstract const type Ta as Wobble super Wibble;

  public function elaborate(
    this::Ta $_,
    this::Ta::Tb $_,
    this::Ta::Tb::Tc $_,
  ): void {}
}

abstract class Wobble {
  abstract const type Tb as Huge super Tiny;
}
abstract class Wibble extends Wobble {
  abstract const type Tb as Bigly super Smol;
}

abstract class Huge {
  abstract const type Tc as MetaHuge super MetaMiniscule;
}
abstract class Bigly extends Huge {
  abstract const type Tc as MetaBigly super MetaMiniscule;
}
abstract class Smol extends Bigly {
  abstract const type Tc as MetaBigly super MetaTiny;
}
abstract class Tiny extends Smol {
  abstract const type Tc as MetaSmol super MetaTiny;
}

abstract class MetaHuge {}
abstract class MetaBigly extends MetaHuge {}
abstract class MetaSmol extends MetaBigly {}
abstract class MetaTiny extends MetaSmol {}
abstract class MetaMiniscule extends MetaTiny {}

function test(): void {
  $fptr = meth_caller(BoundedTyconst::class, 'elaborate');
  hh_expect<
    HH\FunctionRef<(readonly function<
      Ta1 as Wobble with { type Tb = Tb1 } super Wibble with { type Tb = Tb1 },
      Tb1 as Bigly with { type Tc = Tc1 } super Smol with { type Tc = Tc1 },
      Tc1 as MetaSmol super MetaTiny,
    >(BoundedTyconst with { type Ta = Ta1 }, Ta1, Tb1, Tc1): void)>,
  >($fptr);
}

<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTC1 {
  abstract const type TC;
}

abstract class Base1 {
  // this in `as` refinement in `as` constraint
  // this: contrav (receiver) + contrav (as rfmt in as constraint) = contrav → eliminated
  public function as_rfmt_contrav<T as WithTC1 with { type TC as this }>(
    T $_,
  ): void {}

  // this in `super` refinement in `as` constraint
  // this: contrav (receiver) + cov (super rfmt in as constraint) = invariant → kept
  public function super_rfmt_contrav<T as WithTC1 with { type TC super this }>(
    T $_,
  ): void {}

  // this in exact refinement in `as` constraint
  // this: contrav (receiver) + invariant = invariant → kept
  public function exact_rfmt_contrav<T as WithTC1 with { type TC = this }>(
    T $_,
  ): void {}
}

// Same cases but the class has an abstract type constant that is projected
// in the method type, so `this` gets exact refinements in the output
// (e.g., Base1_WithTC with { type TBase = TBase0 })
abstract class Base1_WithTC {
  abstract const type TBase as arraykey;

  // this eliminated → TBase0 kept (invariant via exact rfmt on this)
  public function as_rfmt_contrav<T as WithTC1 with { type TC as this }>(
    T $_,
    this::TBase $_,
  ): void {}

  // this kept → both TBase0 and Tthis kept
  public function super_rfmt_contrav<T as WithTC1 with { type TC super this }>(
    T $_,
    this::TBase $_,
  ): void {}

  // this kept → both TBase0 and Tthis kept
  public function exact_rfmt_contrav<T as WithTC1 with { type TC = this }>(
    T $_,
    this::TBase $_,
  ): void {}
}

function test_type_equality(): void {
  // this eliminated (as rfmt, contrav)
  hh_expect<
    HH\FunctionRef<(readonly function(
      Base1,
      WithTC1 with { type TC as Base1 },
    ): void)>,
  >(meth_caller(Base1::class, 'as_rfmt_contrav'));

  // this kept (super rfmt, invariant)
  hh_expect<
    HH\FunctionRef<(readonly function<Tthis as Base1>(
      Tthis,
      WithTC1 with { type TC super Tthis },
    ): void)>,
  >(meth_caller(Base1::class, 'super_rfmt_contrav'));

  // this kept (exact rfmt, invariant)
  hh_expect<
    HH\FunctionRef<(readonly function<Tthis as Base1>(
      Tthis,
      WithTC1 with { type TC = Tthis },
    ): void)>,
  >(meth_caller(Base1::class, 'exact_rfmt_contrav'));

  // === With class type constants: this gets exact refinements ===

  // this eliminated, TBase0 kept (invariant via exact rfmt on this)
  hh_expect<
    HH\FunctionRef<(readonly function<TBase0 as arraykey>(
      Base1_WithTC with { type TBase = TBase0 },
      WithTC1 with { type TC as Base1_WithTC with { type TBase = TBase0 } },
      TBase0,
    ): void)>,
  >(meth_caller(Base1_WithTC::class, 'as_rfmt_contrav'));

  // this kept, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      Tthis as Base1_WithTC with { type TBase = TBase0 },
    >(
      Tthis,
      WithTC1 with { type TC super Tthis },
      TBase0,
    ): void)>,
  >(meth_caller(Base1_WithTC::class, 'super_rfmt_contrav'));

  // this kept, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      Tthis as Base1_WithTC with { type TBase = TBase0 },
    >(
      Tthis,
      WithTC1 with { type TC = Tthis },
      TBase0,
    ): void)>,
  >(meth_caller(Base1_WithTC::class, 'exact_rfmt_contrav'));
}

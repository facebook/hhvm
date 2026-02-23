<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class Base_Direct1 {
  // this directly in `as` constraint, T contravariant (param only)
  // this: contrav (receiver) + contrav (as constraint) = contrav → eliminated
  // T: contrav (param) → eliminated, T → Base_Direct1
  public function direct_as_contrav<T as this>(T $_): void {}

  // this directly in `super` constraint, T invariant (param + return)
  // this: contrav (receiver) + cov (super constraint) = invariant → kept
  // T: contrav (param) + cov (return) = invariant → kept
  public function direct_super_inv<T super this>(T $_): ?T {
    return null;
  }

  // this directly in `as` constraint, T invariant (param + return)
  // this: contrav (receiver) + contrav (as constraint) = contrav → eliminated
  // T: contrav (param) + cov (return) = invariant → kept, bound simplified
  public function direct_as_inv<T as this>(T $_): ?T {
    return null;
  }
}

// Same cases but the class has an abstract type constant
abstract class Base_Direct1_WithTC {
  abstract const type TBase as arraykey;

  // this eliminated → TBase0 kept (invariant via exact rfmt on this)
  public function direct_as_contrav<T as this>(T $_, this::TBase $_): void {}

  // this kept → both TBase0 and Tthis kept
  public function direct_super_inv<T super this>(
    T $_,
    this::TBase $_,
  ): ?T {
    return null;
  }

  // this eliminated → TBase0 kept
  public function direct_as_inv<T as this>(T $_, this::TBase $_): ?T {
    return null;
  }
}

function test_direct_type_equality(): void {
  // T as this, T contrav: both eliminated
  hh_expect<
    HH\FunctionRef<(readonly function(
      Base_Direct1,
      Base_Direct1,
    ): void)>,
  >(meth_caller(Base_Direct1::class, 'direct_as_contrav'));

  // T super this, T inv: both kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      Tthis as Base_Direct1,
      T super Tthis,
    >(
      Tthis,
      T,
    ): ?T)>,
  >(meth_caller(Base_Direct1::class, 'direct_super_inv'));

  // T as this, T inv: this eliminated, T kept with simplified bound
  hh_expect<
    HH\FunctionRef<(readonly function<T as Base_Direct1>(
      Base_Direct1,
      T,
    ): ?T)>,
  >(meth_caller(Base_Direct1::class, 'direct_as_inv'));

  // === With class type constants: this gets exact refinements ===

  // this eliminated, TBase0 kept (invariant via exact rfmt on this)
  hh_expect<
    HH\FunctionRef<(readonly function<TBase0 as arraykey>(
      Base_Direct1_WithTC with { type TBase = TBase0 },
      Base_Direct1_WithTC with { type TBase = TBase0 },
      TBase0,
    ): void)>,
  >(meth_caller(Base_Direct1_WithTC::class, 'direct_as_contrav'));

  // this kept, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      Tthis as Base_Direct1_WithTC with { type TBase = TBase0 },
      T super Tthis,
    >(
      Tthis,
      T,
      TBase0,
    ): ?T)>,
  >(meth_caller(Base_Direct1_WithTC::class, 'direct_super_inv'));

  // this eliminated, TBase0 kept, T kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      T as Base_Direct1_WithTC with { type TBase = TBase0 },
    >(
      Base_Direct1_WithTC with { type TBase = TBase0 },
      T,
      TBase0,
    ): ?T)>,
  >(meth_caller(Base_Direct1_WithTC::class, 'direct_as_inv'));
}

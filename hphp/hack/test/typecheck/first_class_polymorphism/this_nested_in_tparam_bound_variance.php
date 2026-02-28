<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Cov<+T> {}
class Contrav<-T> {}

abstract class Base_Nested1 {
  // this nested in covariant container in `as` constraint
  // this: contrav (receiver) + contrav (as constraint) * cov (Cov param) = contrav → eliminated
  // T: contrav (param) → eliminated
  public function nested_cov_as<T as Cov<this>>(T $_): void {}

  // this nested in contravariant container in `as` constraint
  // this: contrav (receiver) + contrav (as constraint) * contrav (Contrav param) = cov
  // join(contrav, cov) = invariant → kept
  // T: contrav (param) → eliminated
  public function nested_contrav_as<T as Contrav<this>>(T $_): void {}

  // this nested in covariant container in `super` constraint, T invariant
  // this: contrav (receiver) + cov (super constraint) * cov (Cov param) = cov
  // join(contrav, cov) = invariant → kept
  // T: contrav (param) + cov (return) = invariant → kept
  public function nested_cov_super<T super Cov<this>>(T $_): ?T {
    return null;
  }

  // this nested in contravariant container in `super` constraint, T invariant
  // this: contrav (receiver) + cov (super constraint) * contrav (Contrav param) = contrav
  // join(contrav, contrav) = contrav → eliminated
  // T: contrav (param) + cov (return) = invariant → kept
  public function nested_contrav_super<T super Contrav<this>>(T $_): ?T {
    return null;
  }
}

// Same cases with class type constants
abstract class Base_Nested1_WithTC {
  abstract const type TBase as arraykey;

  public function nested_cov_as<T as Cov<this>>(
    T $_,
    this::TBase $_,
  ): void {}

  public function nested_contrav_as<T as Contrav<this>>(
    T $_,
    this::TBase $_,
  ): void {}

  public function nested_cov_super<T super Cov<this>>(
    T $_,
    this::TBase $_,
  ): ?T {
    return null;
  }

  public function nested_contrav_super<T super Contrav<this>>(
    T $_,
    this::TBase $_,
  ): ?T {
    return null;
  }
}

function test_nested_type_equality(): void {
  // Case 1: Cov in as → this eliminated, T eliminated
  hh_expect<
    HH\FunctionRef<(readonly function(
      Base_Nested1,
      Cov<Base_Nested1>,
    ): void)>,
  >(meth_caller(Base_Nested1::class, 'nested_cov_as'));

  // Case 2: Contrav in as → this kept, T eliminated
  hh_expect<
    HH\FunctionRef<(readonly function<Tthis as Base_Nested1>(
      Tthis,
      Contrav<Tthis>,
    ): void)>,
  >(meth_caller(Base_Nested1::class, 'nested_contrav_as'));

  // Case 3: Cov in super → this kept, T kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      Tthis as Base_Nested1,
      T super Cov<Tthis>,
    >(
      Tthis,
      T,
    ): ?T)>,
  >(meth_caller(Base_Nested1::class, 'nested_cov_super'));

  // Case 4: Contrav in super → this eliminated, T kept
  hh_expect<
    HH\FunctionRef<(readonly function<T super Contrav<Base_Nested1>>(
      Base_Nested1,
      T,
    ): ?T)>,
  >(meth_caller(Base_Nested1::class, 'nested_contrav_super'));

  // === With class type constants ===

  // Case 1 with TC: this eliminated, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<TBase0 as arraykey>(
      Base_Nested1_WithTC with { type TBase = TBase0 },
      Cov<Base_Nested1_WithTC with { type TBase = TBase0 }>,
      TBase0,
    ): void)>,
  >(meth_caller(Base_Nested1_WithTC::class, 'nested_cov_as'));

  // Case 2 with TC: this kept, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      Tthis as Base_Nested1_WithTC with { type TBase = TBase0 },
    >(
      Tthis,
      Contrav<Tthis>,
      TBase0,
    ): void)>,
  >(meth_caller(Base_Nested1_WithTC::class, 'nested_contrav_as'));

  // Case 3 with TC: this kept, T kept, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      Tthis as Base_Nested1_WithTC with { type TBase = TBase0 },
      T super Cov<Tthis>,
    >(
      Tthis,
      T,
      TBase0,
    ): ?T)>,
  >(meth_caller(Base_Nested1_WithTC::class, 'nested_cov_super'));

  // Case 4 with TC: this eliminated, T kept, TBase0 kept
  hh_expect<
    HH\FunctionRef<(readonly function<
      TBase0 as arraykey,
      T super Contrav<Base_Nested1_WithTC with { type TBase = TBase0 }>,
    >(
      Base_Nested1_WithTC with { type TBase = TBase0 },
      T,
      TBase0,
    ): ?T)>,
  >(meth_caller(Base_Nested1_WithTC::class, 'nested_contrav_super'));
}

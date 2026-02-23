<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTC1 {
  abstract const type TC;
}

abstract final class ExistentialVariance {
  // === `as` refinements ===

  // Both eliminated (as rfmt, contrav pos)
  public static function as_rfmt_in_contrav_pos<
    T1 as WithTC1 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): void {}

  // Both eliminated (as rfmt in return type, cov pos)
  public static function as_rfmt_in_cov_pos<
    T2 super arraykey,
  >(): ?(WithTC1 with { type TC as T2 }) {
    return null;
  }

  // T2 eliminated, T1 kept with simplified bound
  public static function as_rfmt_in_inv_pos<
    T1 as WithTC1 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): ?T1 {
    return null;
  }

  // === `super` refinements ===

  // Both eliminated (super rfmt, contrav pos)
  public static function super_rfmt_in_contrav_pos<
    T1 as WithTC1 with { type TC super T2 },
    T2 super int,
  >(T1 $_): void {}

  // Both eliminated (super rfmt in return type, cov pos)
  public static function super_rfmt_in_cov_pos<
    T2 as arraykey,
  >(): ?(WithTC1 with { type TC super T2 }) {
    return null;
  }

  // T2 eliminated, T1 kept with simplified bound
  public static function super_rfmt_in_inv_pos<
    T1 as WithTC1 with { type TC super T2 },
    T2 super int,
  >(T1 $_): ?T1 {
    return null;
  }

  // === Exact `=` refinements (invariant, T2 kept) ===

  // T1 eliminated, T2 kept (exact refinement is invariant)
  public static function exact_rfmt_in_contrav_pos<
    T1 as WithTC1 with { type TC = T2 },
    T2 as arraykey,
  >(T1 $_): void {}

  // === Mixed: T2 in refinement AND in return (invariant, T2 kept) ===

  // T1 eliminated, T2 kept (appears both in refinement and return)
  public static function as_rfmt_tkey_in_return<
    T1 as WithTC1 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): ?T2 {
    return null;
  }
}

function test_type_equality(): void {
  // Both T1 and T2 eliminated (as rfmt, contrav pos)
  hh_expect<
    HH\FunctionRef<(readonly function(
      WithTC1 with { type TC as arraykey },
    ): void)>,
  >(ExistentialVariance::as_rfmt_in_contrav_pos<>);

  // Both eliminated (as rfmt in return type, cov pos)
  hh_expect<
    HH\FunctionRef<(readonly function(): ?(
      WithTC1 with { type TC as arraykey }
    ))>,
  >(ExistentialVariance::as_rfmt_in_cov_pos<>);

  // T2 eliminated, T1 kept with simplified bound
  hh_expect<
    HH\FunctionRef<(readonly function<
      T1 as WithTC1 with { type TC as arraykey },
    >(T1): ?T1)>,
  >(ExistentialVariance::as_rfmt_in_inv_pos<>);

  // Both eliminated (super rfmt, contrav pos)
  hh_expect<
    HH\FunctionRef<(readonly function(
      WithTC1 with { type TC super int },
    ): void)>,
  >(ExistentialVariance::super_rfmt_in_contrav_pos<>);

  // Both eliminated (super rfmt in return type, cov pos)
  hh_expect<
    HH\FunctionRef<(readonly function(): ?(
      WithTC1 with { type TC super arraykey }
    ))>,
  >(ExistentialVariance::super_rfmt_in_cov_pos<>);

  // T2 eliminated, T1 kept with simplified bound
  hh_expect<
    HH\FunctionRef<(readonly function<
      T1 as WithTC1 with { type TC super int },
    >(T1): ?T1)>,
  >(ExistentialVariance::super_rfmt_in_inv_pos<>);

  // T1 eliminated, T2 kept (exact refinement, invariant)
  hh_expect<
    HH\FunctionRef<(readonly function<T2 as arraykey>(
      WithTC1 with { type TC = T2 },
    ): void)>,
  >(ExistentialVariance::exact_rfmt_in_contrav_pos<>);

  // T1 eliminated, T2 kept (invariant: in both refinement and return)
  hh_expect<
    HH\FunctionRef<(readonly function<T2 as arraykey>(
      WithTC1 with { type TC as T2 },
    ): ?T2)>,
  >(ExistentialVariance::as_rfmt_tkey_in_return<>);
}

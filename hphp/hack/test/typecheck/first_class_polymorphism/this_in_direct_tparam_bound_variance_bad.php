<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Unrelated {}

abstract class Base_Direct3 {
  public function direct_as_contrav<T as this>(T $_): void {}

  public function direct_as_inv<T as this>(T $_): ?T {
    return null;
  }
}

final class ConcBase_Direct3 extends Base_Direct3 {}

abstract final class RejectDirect {
  // Simplified direct_as_contrav (both eliminated)
  public static function direct_as_contrav_simp(
    Base_Direct3 $_,
    Base_Direct3 $_,
  ): void {}

  // Simplified direct_as_inv (this eliminated, T kept)
  public static function direct_as_inv_simp<T as Base_Direct3>(
    Base_Direct3 $_,
    T $_,
  ): ?T {
    return null;
  }
}

// === With class type constants ===

abstract class Base_Direct3_WithTC {
  abstract const type TBase as arraykey;

  public function direct_as_contrav<T as this>(T $_, this::TBase $_): void {}

  public function direct_as_inv<T as this>(T $_, this::TBase $_): ?T {
    return null;
  }
}

final class ConcBase_Direct3_WithTC extends Base_Direct3_WithTC {
  const type TBase = int;
}

abstract final class RejectDirect_WithTC {
  public static function direct_as_contrav_simp<TBase0 as arraykey>(
    Base_Direct3_WithTC with { type TBase = TBase0 } $_,
    Base_Direct3_WithTC with { type TBase = TBase0 } $_,
    TBase0 $_,
  ): void {}

  public static function direct_as_inv_simp<
    TBase0 as arraykey,
    T as Base_Direct3_WithTC with { type TBase = TBase0 },
  >(
    Base_Direct3_WithTC with { type TBase = TBase0 } $_,
    T $_,
    TBase0 $_,
  ): ?T {
    return null;
  }
}

function test_reject(): void {
  // direct_as_contrav: Unrelated is NOT <: ConcBase_Direct3 → rejected
  (new ConcBase_Direct3())->direct_as_contrav(new Unrelated());
  RejectDirect::direct_as_contrav_simp(
    new ConcBase_Direct3(),
    new Unrelated(),
  );

  // direct_as_inv: Unrelated is NOT <: ConcBase_Direct3 → rejected
  (new ConcBase_Direct3())->direct_as_inv(new Unrelated());
  RejectDirect::direct_as_inv_simp(
    new ConcBase_Direct3(),
    new Unrelated(),
  );

  // === With class type constants ===

  (new ConcBase_Direct3_WithTC())->direct_as_contrav(new Unrelated(), 0);
  RejectDirect_WithTC::direct_as_contrav_simp(
    new ConcBase_Direct3_WithTC(),
    new Unrelated(),
    0,
  );

  (new ConcBase_Direct3_WithTC())->direct_as_inv(new Unrelated(), 0);
  RejectDirect_WithTC::direct_as_inv_simp(
    new ConcBase_Direct3_WithTC(),
    new Unrelated(),
    0,
  );
}

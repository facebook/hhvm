<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class Base_Direct2 {
  public function direct_as_contrav<T as this>(T $_): void {}

  public function direct_super_inv<T super this>(T $_): ?T {
    return null;
  }

  public function direct_as_inv<T as this>(T $_): ?T {
    return null;
  }
}

final class ConcBase_Direct2 extends Base_Direct2 {}

abstract final class AcceptDirect {
  // Simplified direct_as_contrav (both eliminated)
  public static function direct_as_contrav_simp(
    Base_Direct2 $_,
    Base_Direct2 $_,
  ): void {}

  // Simplified direct_super_inv (both kept)
  public static function direct_super_inv_simp<
    Tthis as Base_Direct2,
    T super Tthis,
  >(Tthis $_, T $_): ?T {
    return null;
  }

  // Simplified direct_as_inv (this eliminated, T kept)
  public static function direct_as_inv_simp<T as Base_Direct2>(
    Base_Direct2 $_,
    T $_,
  ): ?T {
    return null;
  }
}

// === With class type constants ===

abstract class Base_Direct2_WithTC {
  abstract const type TBase as arraykey;

  public function direct_as_contrav<T as this>(T $_, this::TBase $_): void {}

  public function direct_super_inv<T super this>(
    T $_,
    this::TBase $_,
  ): ?T {
    return null;
  }

  public function direct_as_inv<T as this>(T $_, this::TBase $_): ?T {
    return null;
  }
}

final class ConcBase_Direct2_WithTC extends Base_Direct2_WithTC {
  const type TBase = int;
}

abstract final class AcceptDirect_WithTC {
  // Simplified direct_as_contrav (this eliminated, TBase0 kept)
  public static function direct_as_contrav_simp<TBase0 as arraykey>(
    Base_Direct2_WithTC with { type TBase = TBase0 } $_,
    Base_Direct2_WithTC with { type TBase = TBase0 } $_,
    TBase0 $_,
  ): void {}

  // Simplified direct_super_inv (this kept, TBase0 kept)
  public static function direct_super_inv_simp<
    TBase0 as arraykey,
    Tthis as Base_Direct2_WithTC with { type TBase = TBase0 },
    T super Tthis,
  >(
    Tthis $_,
    T $_,
    TBase0 $_,
  ): ?T {
    return null;
  }

  // Simplified direct_as_inv (this eliminated, TBase0 kept, T kept)
  public static function direct_as_inv_simp<
    TBase0 as arraykey,
    T as Base_Direct2_WithTC with { type TBase = TBase0 },
  >(
    Base_Direct2_WithTC with { type TBase = TBase0 } $_,
    T $_,
    TBase0 $_,
  ): ?T {
    return null;
  }
}

function test_accept(): void {
  // direct_as_contrav: ConcBase_Direct2 <: Base_Direct2, T=ConcBase_Direct2 as this ✓
  (new ConcBase_Direct2())->direct_as_contrav(new ConcBase_Direct2());
  AcceptDirect::direct_as_contrav_simp(
    new ConcBase_Direct2(),
    new ConcBase_Direct2(),
  );

  // direct_super_inv: T=ConcBase_Direct2, ConcBase_Direct2 super ConcBase_Direct2 ✓
  (new ConcBase_Direct2())->direct_super_inv(new ConcBase_Direct2());
  AcceptDirect::direct_super_inv_simp(
    new ConcBase_Direct2(),
    new ConcBase_Direct2(),
  );

  // direct_as_inv: T=ConcBase_Direct2, ConcBase_Direct2 as ConcBase_Direct2 ✓
  (new ConcBase_Direct2())->direct_as_inv(new ConcBase_Direct2());
  AcceptDirect::direct_as_inv_simp(
    new ConcBase_Direct2(),
    new ConcBase_Direct2(),
  );

  // === With class type constants ===

  (new ConcBase_Direct2_WithTC())->direct_as_contrav(
    new ConcBase_Direct2_WithTC(),
    0,
  );
  AcceptDirect_WithTC::direct_as_contrav_simp(
    new ConcBase_Direct2_WithTC(),
    new ConcBase_Direct2_WithTC(),
    0,
  );

  (new ConcBase_Direct2_WithTC())->direct_super_inv(
    new ConcBase_Direct2_WithTC(),
    0,
  );
  AcceptDirect_WithTC::direct_super_inv_simp(
    new ConcBase_Direct2_WithTC(),
    new ConcBase_Direct2_WithTC(),
    0,
  );

  (new ConcBase_Direct2_WithTC())->direct_as_inv(
    new ConcBase_Direct2_WithTC(),
    0,
  );
  AcceptDirect_WithTC::direct_as_inv_simp(
    new ConcBase_Direct2_WithTC(),
    new ConcBase_Direct2_WithTC(),
    0,
  );
}

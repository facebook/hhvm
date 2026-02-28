<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTC2 {
  abstract const type TC;
}

abstract class Base2 {
  // this in `as` refinement → eliminated, simplified to (Base2, WithTC2 with { type TC as Base2 }): void
  public function as_rfmt_contrav<T as WithTC2 with { type TC as this }>(
    T $_,
  ): void {}

  // this in `super` refinement → kept, simplified to <Tthis as Base2>(Tthis, WithTC2 with { type TC super Tthis }): void
  public function super_rfmt_contrav<
    T as WithTC2 with { type TC super this },
  >(T $_): void {}

  // this in exact refinement → kept, simplified to <Tthis as Base2>(Tthis, WithTC2 with { type TC = Tthis }): void
  public function exact_rfmt_contrav<T as WithTC2 with { type TC = this }>(
    T $_,
  ): void {}
}

final class ConcBase2_Sub extends Base2 {}

class WithTC2_ConcBase2Sub extends WithTC2 {
  const type TC = ConcBase2_Sub;
}

class WithTC2_Mixed extends WithTC2 {
  const type TC = mixed;
}

abstract final class AcceptSimplified {
  // Simplified version of as_rfmt_contrav (this eliminated)
  public static function as_rfmt_contrav_simp(
    Base2 $_,
    WithTC2 with { type TC as Base2 } $_,
  ): void {}

  // Simplified version of super_rfmt_contrav (this kept)
  public static function super_rfmt_contrav_simp<Tthis as Base2>(
    Tthis $_,
    WithTC2 with { type TC super Tthis } $_,
  ): void {}

  // Simplified version of exact_rfmt_contrav (this kept)
  public static function exact_rfmt_contrav_simp<Tthis as Base2>(
    Tthis $_,
    WithTC2 with { type TC = Tthis } $_,
  ): void {}
}

// === With class type constants: this gets exact refinements ===

abstract class Base2_WithTC {
  abstract const type TBase as arraykey;

  // this eliminated → TBase0 kept (invariant via exact rfmt on this)
  public function as_rfmt_contrav<T as WithTC2 with { type TC as this }>(
    T $_,
    this::TBase $_,
  ): void {}

  // this kept → both TBase0 and Tthis kept
  public function super_rfmt_contrav<T as WithTC2 with { type TC super this }>(
    T $_,
    this::TBase $_,
  ): void {}

  // this kept → both TBase0 and Tthis kept
  public function exact_rfmt_contrav<T as WithTC2 with { type TC = this }>(
    T $_,
    this::TBase $_,
  ): void {}
}

final class ConcBase2_WithTC extends Base2_WithTC {
  const type TBase = int;
}

class WithTC2_ConcBase2WithTC extends WithTC2 {
  const type TC = ConcBase2_WithTC;
}

abstract final class AcceptSimplified_WithTC {
  // Simplified as_rfmt_contrav (this eliminated, TBase0 kept)
  public static function as_rfmt_contrav_simp<TBase0 as arraykey>(
    Base2_WithTC with { type TBase = TBase0 } $_,
    WithTC2 with { type TC as Base2_WithTC with { type TBase = TBase0 } } $_,
    TBase0 $_,
  ): void {}

  // Simplified super_rfmt_contrav (this kept, TBase0 kept)
  public static function super_rfmt_contrav_simp<
    TBase0 as arraykey,
    Tthis as Base2_WithTC with { type TBase = TBase0 },
  >(
    Tthis $_,
    WithTC2 with { type TC super Tthis } $_,
    TBase0 $_,
  ): void {}

  // Simplified exact_rfmt_contrav (this kept, TBase0 kept)
  public static function exact_rfmt_contrav_simp<
    TBase0 as arraykey,
    Tthis as Base2_WithTC with { type TBase = TBase0 },
  >(
    Tthis $_,
    WithTC2 with { type TC = Tthis } $_,
    TBase0 $_,
  ): void {}
}

function test_accept(): void {
  // as_rfmt: WithTC2_ConcBase2Sub has TC=ConcBase2_Sub <: Base2 ✓
  (new ConcBase2_Sub())->as_rfmt_contrav(new WithTC2_ConcBase2Sub());
  AcceptSimplified::as_rfmt_contrav_simp(
    new ConcBase2_Sub(),
    new WithTC2_ConcBase2Sub(),
  );

  // super_rfmt: WithTC2_Mixed has TC=mixed, mixed super ConcBase2_Sub ✓
  (new ConcBase2_Sub())->super_rfmt_contrav(new WithTC2_Mixed());
  AcceptSimplified::super_rfmt_contrav_simp(
    new ConcBase2_Sub(),
    new WithTC2_Mixed(),
  );

  // exact_rfmt: WithTC2_ConcBase2Sub has TC=ConcBase2_Sub = Tthis ✓
  (new ConcBase2_Sub())->exact_rfmt_contrav(new WithTC2_ConcBase2Sub());
  AcceptSimplified::exact_rfmt_contrav_simp(
    new ConcBase2_Sub(),
    new WithTC2_ConcBase2Sub(),
  );

  // === With class type constants ===

  // as_rfmt: WithTC2_ConcBase2WithTC has TC=ConcBase2_WithTC <: ConcBase2_WithTC ✓
  (new ConcBase2_WithTC())->as_rfmt_contrav(new WithTC2_ConcBase2WithTC(), 0);
  AcceptSimplified_WithTC::as_rfmt_contrav_simp(
    new ConcBase2_WithTC(),
    new WithTC2_ConcBase2WithTC(),
    0,
  );

  // super_rfmt: WithTC2_Mixed has TC=mixed, mixed super ConcBase2_WithTC ✓
  (new ConcBase2_WithTC())->super_rfmt_contrav(new WithTC2_Mixed(), 0);
  AcceptSimplified_WithTC::super_rfmt_contrav_simp(
    new ConcBase2_WithTC(),
    new WithTC2_Mixed(),
    0,
  );

  // exact_rfmt: WithTC2_ConcBase2WithTC has TC=ConcBase2_WithTC = Tthis ✓
  (new ConcBase2_WithTC())->exact_rfmt_contrav(new WithTC2_ConcBase2WithTC(), 0);
  AcceptSimplified_WithTC::exact_rfmt_contrav_simp(
    new ConcBase2_WithTC(),
    new WithTC2_ConcBase2WithTC(),
    0,
  );
}

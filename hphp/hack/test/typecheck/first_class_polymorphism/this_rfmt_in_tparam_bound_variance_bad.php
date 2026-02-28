<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTC3 {
  abstract const type TC;
}

class WithTC3_Mixed extends WithTC3 {
  const type TC = mixed;
}

class WithTC3_Int extends WithTC3 {
  const type TC = int;
}

abstract class Base3 {
  // this in `as` refinement → eliminated, simplified to (Base3, WithTC3 with { type TC as Base3 }): void
  public function as_rfmt_contrav<T as WithTC3 with { type TC as this }>(
    T $_,
  ): void {}

  // this in `super` refinement → kept, simplified to <Tthis as Base3>(Tthis, WithTC3 with { type TC super Tthis }): void
  public function super_rfmt_contrav<
    T as WithTC3 with { type TC super this },
  >(T $_): void {}

  // this in exact refinement → kept, simplified to <Tthis as Base3>(Tthis, WithTC3 with { type TC = Tthis }): void
  public function exact_rfmt_contrav<T as WithTC3 with { type TC = this }>(
    T $_,
  ): void {}
}

final class ConcBase3 extends Base3 {}

abstract final class RejectSimplified {
  // Simplified version of as_rfmt_contrav (this eliminated)
  public static function as_rfmt_contrav_simp(
    Base3 $_,
    WithTC3 with { type TC as Base3 } $_,
  ): void {}

  // Simplified version of super_rfmt_contrav (this kept)
  public static function super_rfmt_contrav_simp<Tthis as Base3>(
    Tthis $_,
    WithTC3 with { type TC super Tthis } $_,
  ): void {}

  // Simplified version of exact_rfmt_contrav (this kept)
  public static function exact_rfmt_contrav_simp<Tthis as Base3>(
    Tthis $_,
    WithTC3 with { type TC = Tthis } $_,
  ): void {}
}

// === With class type constants ===

abstract class Base3_WithTC {
  abstract const type TBase as arraykey;

  public function as_rfmt_contrav<T as WithTC3 with { type TC as this }>(
    T $_,
    this::TBase $_,
  ): void {}

  public function super_rfmt_contrav<T as WithTC3 with { type TC super this }>(
    T $_,
    this::TBase $_,
  ): void {}

  public function exact_rfmt_contrav<T as WithTC3 with { type TC = this }>(
    T $_,
    this::TBase $_,
  ): void {}
}

final class ConcBase3_WithTC extends Base3_WithTC {
  const type TBase = int;
}

abstract final class RejectSimplified_WithTC {
  // Simplified as_rfmt_contrav (this eliminated, TBase0 kept)
  public static function as_rfmt_contrav_simp<TBase0 as arraykey>(
    Base3_WithTC with { type TBase = TBase0 } $_,
    WithTC3 with { type TC as Base3_WithTC with { type TBase = TBase0 } } $_,
    TBase0 $_,
  ): void {}

  // Simplified super_rfmt_contrav (this kept, TBase0 kept)
  public static function super_rfmt_contrav_simp<
    TBase0 as arraykey,
    Tthis as Base3_WithTC with { type TBase = TBase0 },
  >(
    Tthis $_,
    WithTC3 with { type TC super Tthis } $_,
    TBase0 $_,
  ): void {}

  // Simplified exact_rfmt_contrav (this kept, TBase0 kept)
  public static function exact_rfmt_contrav_simp<
    TBase0 as arraykey,
    Tthis as Base3_WithTC with { type TBase = TBase0 },
  >(
    Tthis $_,
    WithTC3 with { type TC = Tthis } $_,
    TBase0 $_,
  ): void {}
}

function test_reject(): void {
  // as_rfmt: WithTC3_Mixed has TC=mixed, mixed is NOT <: ConcBase3 → rejected
  (new ConcBase3())->as_rfmt_contrav(new WithTC3_Mixed());
  RejectSimplified::as_rfmt_contrav_simp(
    new ConcBase3(),
    new WithTC3_Mixed(),
  );

  // super_rfmt: WithTC3_Int has TC=int, int is NOT super ConcBase3 → rejected
  (new ConcBase3())->super_rfmt_contrav(new WithTC3_Int());
  RejectSimplified::super_rfmt_contrav_simp(
    new ConcBase3(),
    new WithTC3_Int(),
  );

  // exact_rfmt: WithTC3_Int has TC=int, int ≠ ConcBase3 → rejected
  (new ConcBase3())->exact_rfmt_contrav(new WithTC3_Int());
  RejectSimplified::exact_rfmt_contrav_simp(
    new ConcBase3(),
    new WithTC3_Int(),
  );

  // === With class type constants ===

  // as_rfmt: WithTC3_Mixed has TC=mixed, mixed is NOT <: ConcBase3_WithTC → rejected
  (new ConcBase3_WithTC())->as_rfmt_contrav(new WithTC3_Mixed(), 0);
  RejectSimplified_WithTC::as_rfmt_contrav_simp(
    new ConcBase3_WithTC(),
    new WithTC3_Mixed(),
    0,
  );

  // super_rfmt: WithTC3_Int has TC=int, int is NOT super ConcBase3_WithTC → rejected
  (new ConcBase3_WithTC())->super_rfmt_contrav(new WithTC3_Int(), 0);
  RejectSimplified_WithTC::super_rfmt_contrav_simp(
    new ConcBase3_WithTC(),
    new WithTC3_Int(),
    0,
  );

  // exact_rfmt: WithTC3_Int has TC=int, int ≠ ConcBase3_WithTC → rejected
  (new ConcBase3_WithTC())->exact_rfmt_contrav(new WithTC3_Int(), 0);
  RejectSimplified_WithTC::exact_rfmt_contrav_simp(
    new ConcBase3_WithTC(),
    new WithTC3_Int(),
    0,
  );
}

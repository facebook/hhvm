<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Cov2<+T> {}
class Contrav2<-T> {}

abstract class Base_Nested2 {
  public function nested_cov_as<T as Cov2<this>>(T $_): void {}

  public function nested_contrav_as<T as Contrav2<this>>(T $_): void {}

  public function nested_cov_super<T super Cov2<this>>(T $_): ?T {
    return null;
  }

  public function nested_contrav_super<T super Contrav2<this>>(T $_): ?T {
    return null;
  }
}

final class ConcBase_Nested2 extends Base_Nested2 {}

// Factory functions to produce values of specific types
function cov2_concbase(): Cov2<ConcBase_Nested2> {
  throw new Exception();
}
function contrav2_base(): Contrav2<Base_Nested2> {
  throw new Exception();
}
function cov2_base(): Cov2<Base_Nested2> {
  throw new Exception();
}
function contrav2_concbase(): Contrav2<ConcBase_Nested2> {
  throw new Exception();
}

abstract final class AcceptNested {
  // Simplified nested_cov_as (both eliminated)
  public static function nested_cov_as_simp(
    Base_Nested2 $_,
    Cov2<Base_Nested2> $_,
  ): void {}

  // Simplified nested_contrav_as (this kept, T eliminated)
  public static function nested_contrav_as_simp<Tthis as Base_Nested2>(
    Tthis $_,
    Contrav2<Tthis> $_,
  ): void {}

  // Simplified nested_cov_super (this kept, T kept)
  public static function nested_cov_super_simp<
    Tthis as Base_Nested2,
    T super Cov2<Tthis>,
  >(Tthis $_, T $_): ?T {
    return null;
  }

  // Simplified nested_contrav_super (this eliminated, T kept)
  public static function nested_contrav_super_simp<
    T super Contrav2<Base_Nested2>,
  >(Base_Nested2 $_, T $_): ?T {
    return null;
  }
}

function test_accept(): void {
  // nested_cov_as: Cov2<ConcBase_Nested2> <: Cov2<ConcBase_Nested2> ✓
  (new ConcBase_Nested2())->nested_cov_as(cov2_concbase());
  AcceptNested::nested_cov_as_simp(new ConcBase_Nested2(), cov2_concbase());

  // nested_contrav_as: Contrav2<Base_Nested2> <: Contrav2<ConcBase_Nested2> ✓
  // (Contrav2 contravariant, ConcBase_Nested2 <: Base_Nested2)
  (new ConcBase_Nested2())->nested_contrav_as(contrav2_base());
  AcceptNested::nested_contrav_as_simp(
    new ConcBase_Nested2(),
    contrav2_base(),
  );

  // nested_cov_super: T=Cov2<Base_Nested2>, Cov2<ConcBase_Nested2> <: Cov2<Base_Nested2> ✓
  // (Cov2 covariant, ConcBase_Nested2 <: Base_Nested2)
  (new ConcBase_Nested2())->nested_cov_super(cov2_base());
  AcceptNested::nested_cov_super_simp(new ConcBase_Nested2(), cov2_base());

  // nested_contrav_super: T=Contrav2<ConcBase_Nested2>,
  // Contrav2<ConcBase_Nested2> <: Contrav2<ConcBase_Nested2> ✓
  (new ConcBase_Nested2())->nested_contrav_super(contrav2_concbase());
  AcceptNested::nested_contrav_super_simp(
    new ConcBase_Nested2(),
    contrav2_concbase(),
  );
}

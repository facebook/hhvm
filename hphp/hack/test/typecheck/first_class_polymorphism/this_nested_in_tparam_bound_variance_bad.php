<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class Cov3<+T> {}
class Contrav3<-T> {}

abstract class Base_Nested3 {
  public function nested_cov_as<T as Cov3<this>>(T $_): void {}

  public function nested_contrav_as<T as Contrav3<this>>(T $_): void {}
}

final class ConcBase_Nested3 extends Base_Nested3 {}

// Factory functions to produce values that should be REJECTED
function cov3_int(): Cov3<int> {
  throw new Exception();
}
function contrav3_int(): Contrav3<int> {
  throw new Exception();
}

abstract final class RejectNested {
  // Simplified nested_cov_as (both eliminated)
  public static function nested_cov_as_simp(
    Base_Nested3 $_,
    Cov3<Base_Nested3> $_,
  ): void {}

  // Simplified nested_contrav_as (this kept, T eliminated)
  public static function nested_contrav_as_simp<Tthis as Base_Nested3>(
    Tthis $_,
    Contrav3<Tthis> $_,
  ): void {}
}

function test_reject(): void {
  // nested_cov_as: Cov3<int> is NOT <: Cov3<ConcBase_Nested3> → rejected
  // (int is not <: ConcBase_Nested3)
  (new ConcBase_Nested3())->nested_cov_as(cov3_int());
  RejectNested::nested_cov_as_simp(new ConcBase_Nested3(), cov3_int());

  // nested_contrav_as: Contrav3<int> is NOT <: Contrav3<ConcBase_Nested3> → rejected
  // (would need ConcBase_Nested3 <: int, which is false)
  (new ConcBase_Nested3())->nested_contrav_as(contrav3_int());
  RejectNested::nested_contrav_as_simp(new ConcBase_Nested3(), contrav3_int());
}

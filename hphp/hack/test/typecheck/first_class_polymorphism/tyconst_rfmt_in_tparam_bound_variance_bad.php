<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTC3 {
  abstract const type TC;
}

class ConcreteTC3_Mixed extends WithTC3 {
  const type TC = mixed;
}

class ConcreteTC3_String extends WithTC3 {
  const type TC = string;
}

// Factory functions to produce values that should be REJECTED
function tc3_mixed(): ConcreteTC3_Mixed {
  throw new Exception();
}
function tc3_string(): ConcreteTC3_String {
  throw new Exception();
}

abstract final class ExistentialVarianceReject {

  // Simplified: (WithTC3 with { type TC as arraykey }): void
  public static function as_rfmt_in_contrav_pos<
    T1 as WithTC3 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): void {}

  public static function as_rfmt_in_contrav_pos_simp(
    WithTC3 with { type TC as arraykey } $_,
  ): void {}

  // Simplified: <T1 as WithTC3 with { type TC as arraykey }>(T1): ?T1
  public static function as_rfmt_in_inv_pos<
    T1 as WithTC3 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): ?T1 {
    return null;
  }

  public static function as_rfmt_in_inv_pos_simp<
    T1 as WithTC3 with { type TC as arraykey },
  >(T1 $_): ?T1 {
    return null;
  }

  // Simplified: (WithTC3 with { type TC super int }): void
  public static function super_rfmt_in_contrav_pos<
    T1 as WithTC3 with { type TC super T2 },
    T2 super int,
  >(T1 $_): void {}

  public static function super_rfmt_in_contrav_pos_simp(
    WithTC3 with { type TC super int } $_,
  ): void {}

  // Simplified: <T1 as WithTC3 with { type TC super int }>(T1): ?T1
  public static function super_rfmt_in_inv_pos<
    T1 as WithTC3 with { type TC super T2 },
    T2 super int,
  >(T1 $_): ?T1 {
    return null;
  }

  public static function super_rfmt_in_inv_pos_simp<
    T1 as WithTC3 with { type TC super int },
  >(T1 $_): ?T1 {
    return null;
  }

  // Simplified: <T2 as arraykey>(WithTC3 with { type TC = T2 }): void
  public static function exact_rfmt_in_contrav_pos<
    T1 as WithTC3 with { type TC = T2 },
    T2 as arraykey,
  >(T1 $_): void {}

  public static function exact_rfmt_in_contrav_pos_simp<T2 as arraykey>(
    WithTC3 with { type TC = T2 } $_,
  ): void {}

  // Simplified: <T2 as arraykey>(WithTC3 with { type TC as T2 }): ?T2
  public static function as_rfmt_tkey_in_return<
    T1 as WithTC3 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): ?T2 {
    return null;
  }

  public static function as_rfmt_tkey_in_return_simp<T2 as arraykey>(
    WithTC3 with { type TC as T2 } $_,
  ): ?T2 {
    return null;
  }
}

function test_reject(): void {
  // ConcreteTC3_Mixed rejected: mixed is not <: arraykey
  ExistentialVarianceReject::as_rfmt_in_contrav_pos(tc3_mixed());
  ExistentialVarianceReject::as_rfmt_in_contrav_pos_simp(tc3_mixed());

  // ConcreteTC3_Mixed rejected: mixed is not <: arraykey
  ExistentialVarianceReject::as_rfmt_in_inv_pos(tc3_mixed());
  ExistentialVarianceReject::as_rfmt_in_inv_pos_simp(tc3_mixed());

  // ConcreteTC3_String rejected: string is not >: int
  ExistentialVarianceReject::super_rfmt_in_contrav_pos(tc3_string());
  ExistentialVarianceReject::super_rfmt_in_contrav_pos_simp(tc3_string());

  // ConcreteTC3_String rejected: string is not >: int
  ExistentialVarianceReject::super_rfmt_in_inv_pos(tc3_string());
  ExistentialVarianceReject::super_rfmt_in_inv_pos_simp(tc3_string());

  // ConcreteTC3_Mixed rejected: T2=mixed, mixed is not <: arraykey
  ExistentialVarianceReject::exact_rfmt_in_contrav_pos(tc3_mixed());
  ExistentialVarianceReject::exact_rfmt_in_contrav_pos_simp(tc3_mixed());

  // ConcreteTC3_Mixed rejected: mixed is not <: arraykey
  ExistentialVarianceReject::as_rfmt_tkey_in_return(tc3_mixed());
  ExistentialVarianceReject::as_rfmt_tkey_in_return_simp(tc3_mixed());
}

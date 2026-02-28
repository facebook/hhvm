<?hh
<<file: __EnableUnstableFeatures('type_const_super_bound')>>
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class WithTC2 {
  abstract const type TC;
}

class ConcreteTC2_Int extends WithTC2 {
  const type TC = int;
}

class ConcreteTC2_Arraykey extends WithTC2 {
  const type TC = arraykey;
}

// Factory functions to produce values of specific types
function tc2_int(): ConcreteTC2_Int {
  throw new Exception();
}
function tc2_arraykey(): ConcreteTC2_Arraykey {
  throw new Exception();
}

abstract final class ExistentialVarianceAccept {

  // Simplified: (WithTC2 with { type TC as arraykey }): void
  public static function as_rfmt_in_contrav_pos<
    T1 as WithTC2 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): void {}

  public static function as_rfmt_in_contrav_pos_simp(
    WithTC2 with { type TC as arraykey } $_,
  ): void {}

  // Simplified: <T1 as WithTC2 with { type TC as arraykey }>(T1): ?T1
  public static function as_rfmt_in_inv_pos<
    T1 as WithTC2 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): ?T1 {
    return null;
  }

  public static function as_rfmt_in_inv_pos_simp<
    T1 as WithTC2 with { type TC as arraykey },
  >(T1 $_): ?T1 {
    return null;
  }

  // Simplified: (WithTC2 with { type TC super int }): void
  public static function super_rfmt_in_contrav_pos<
    T1 as WithTC2 with { type TC super T2 },
    T2 super int,
  >(T1 $_): void {}

  public static function super_rfmt_in_contrav_pos_simp(
    WithTC2 with { type TC super int } $_,
  ): void {}

  // Simplified: <T1 as WithTC2 with { type TC super int }>(T1): ?T1
  public static function super_rfmt_in_inv_pos<
    T1 as WithTC2 with { type TC super T2 },
    T2 super int,
  >(T1 $_): ?T1 {
    return null;
  }

  public static function super_rfmt_in_inv_pos_simp<
    T1 as WithTC2 with { type TC super int },
  >(T1 $_): ?T1 {
    return null;
  }

  // Simplified: <T2 as arraykey>(WithTC2 with { type TC = T2 }): void
  public static function exact_rfmt_in_contrav_pos<
    T1 as WithTC2 with { type TC = T2 },
    T2 as arraykey,
  >(T1 $_): void {}

  public static function exact_rfmt_in_contrav_pos_simp<T2 as arraykey>(
    WithTC2 with { type TC = T2 } $_,
  ): void {}

  // Simplified: <T2 as arraykey>(WithTC2 with { type TC as T2 }): ?T2
  public static function as_rfmt_tkey_in_return<
    T1 as WithTC2 with { type TC as T2 },
    T2 as arraykey,
  >(T1 $_): ?T2 {
    return null;
  }

  public static function as_rfmt_tkey_in_return_simp<T2 as arraykey>(
    WithTC2 with { type TC as T2 } $_,
  ): ?T2 {
    return null;
  }
}

function test_accept(): void {
  // ConcreteTC2_Int accepted (int <: arraykey)
  ExistentialVarianceAccept::as_rfmt_in_contrav_pos(tc2_int());
  ExistentialVarianceAccept::as_rfmt_in_contrav_pos_simp(tc2_int());

  // ConcreteTC2_Int accepted (int <: arraykey)
  ExistentialVarianceAccept::as_rfmt_in_inv_pos(tc2_int());
  ExistentialVarianceAccept::as_rfmt_in_inv_pos_simp(tc2_int());

  // ConcreteTC2_Arraykey accepted (arraykey >: int)
  ExistentialVarianceAccept::super_rfmt_in_contrav_pos(tc2_arraykey());
  ExistentialVarianceAccept::super_rfmt_in_contrav_pos_simp(tc2_arraykey());

  // ConcreteTC2_Arraykey accepted (arraykey >: int)
  ExistentialVarianceAccept::super_rfmt_in_inv_pos(tc2_arraykey());
  ExistentialVarianceAccept::super_rfmt_in_inv_pos_simp(tc2_arraykey());

  // ConcreteTC2_Int accepted (TC=int, T2=int, int <: arraykey)
  ExistentialVarianceAccept::exact_rfmt_in_contrav_pos(tc2_int());
  ExistentialVarianceAccept::exact_rfmt_in_contrav_pos_simp(tc2_int());

  // ConcreteTC2_Int accepted (TC=int, int <: T2, T2=int <: arraykey)
  ExistentialVarianceAccept::as_rfmt_tkey_in_return(tc2_int());
  ExistentialVarianceAccept::as_rfmt_tkey_in_return_simp(tc2_int());
}

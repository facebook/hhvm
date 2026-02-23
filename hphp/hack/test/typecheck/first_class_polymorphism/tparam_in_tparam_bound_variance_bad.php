<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

final class Inv3<T> {}
final class Cov3<+T> {}
final class Contrav3<-T> {}

// Factory functions to produce values that should be REJECTED
function cov3_mixed(): Cov3<mixed> {
  throw new Exception();
}
function contrav3_int(): Contrav3<int> {
  throw new Exception();
}
function inv3_int(): Inv3<int> {
  throw new Exception();
}
function cov3_contrav_int(): Cov3<Contrav3<int>> {
  throw new Exception();
}
function contrav3_contrav_mixed(): Contrav3<Contrav3<mixed>> {
  throw new Exception();
}

abstract final class VarianceReject {

  // Simplified: (Cov3<arraykey>): void
  public static function cov_class_in_contrav_pos<
    T as Cov3<TKey>,
    TKey as arraykey,
  >(T $_): void {}

  public static function cov_class_in_contrav_pos_simp(
    Cov3<arraykey> $_,
  ): void {}

  // Simplified: <T as Cov3<arraykey>>(T): ?T
  public static function cov_class_in_inv_pos<
    T as Cov3<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function cov_class_in_inv_pos_simp<T as Cov3<arraykey>>(
    T $_,
  ): ?T {
    return null;
  }

  // Simplified: (Contrav3<arraykey>): void
  public static function contrav_class_in_contrav_pos<
    T as Contrav3<TKey>,
    TKey super arraykey,
  >(T $_): void {}

  public static function contrav_class_in_contrav_pos_simp(
    Contrav3<arraykey> $_,
  ): void {}

  // Simplified: <TKey super arraykey>(Inv3<TKey>): void
  public static function inv_class_in_contrav_pos<
    T as Inv3<TKey>,
    TKey super arraykey,
  >(T $_): void {}

  public static function inv_class_in_contrav_pos_simp<TKey super arraykey>(
    Inv3<TKey> $_,
  ): void {}

  // Simplified: <TKey as arraykey>(Cov3<TKey>): ?TKey
  public static function tkey_in_bound_and_return<
    T as Cov3<TKey>,
    TKey as arraykey,
  >(T $_): ?TKey {
    return null;
  }

  public static function tkey_in_bound_and_return_simp<TKey as arraykey>(
    Cov3<TKey> $_,
  ): ?TKey {
    return null;
  }

  // Simplified: (Cov3<Contrav3<arraykey>>): void
  public static function nested_cov_contrav<
    T as Cov3<Contrav3<TKey>>,
    TKey super arraykey,
  >(T $_): void {}

  public static function nested_cov_contrav_simp(
    Cov3<Contrav3<arraykey>> $_,
  ): void {}

  // Simplified: (Contrav3<Contrav3<arraykey>>): void
  public static function nested_contrav_contrav<
    T as Contrav3<Contrav3<TKey>>,
    TKey as arraykey,
  >(T $_): void {}

  public static function nested_contrav_contrav_simp(
    Contrav3<Contrav3<arraykey>> $_,
  ): void {}
}

function test_reject(): void {
  // Cov3<mixed> rejected: mixed is not <: arraykey
  VarianceReject::cov_class_in_contrav_pos(cov3_mixed());
  VarianceReject::cov_class_in_contrav_pos_simp(cov3_mixed());

  // Cov3<mixed> rejected: Cov3<mixed> is not <: Cov3<arraykey>
  VarianceReject::cov_class_in_inv_pos(cov3_mixed());
  VarianceReject::cov_class_in_inv_pos_simp(cov3_mixed());

  // Contrav3<int> rejected: int is not super arraykey (arraykey = int|string)
  VarianceReject::contrav_class_in_contrav_pos(contrav3_int());
  VarianceReject::contrav_class_in_contrav_pos_simp(contrav3_int());

  // Inv3<int> rejected: int is not super arraykey
  VarianceReject::inv_class_in_contrav_pos(inv3_int());
  VarianceReject::inv_class_in_contrav_pos_simp(inv3_int());

  // Cov3<mixed> rejected: mixed is not <: arraykey
  VarianceReject::tkey_in_bound_and_return(cov3_mixed());
  VarianceReject::tkey_in_bound_and_return_simp(cov3_mixed());

  // Cov3<Contrav3<int>> rejected: Contrav3<int> is not <: Contrav3<arraykey>
  // (would need arraykey <: int, which is false)
  VarianceReject::nested_cov_contrav(cov3_contrav_int());
  VarianceReject::nested_cov_contrav_simp(cov3_contrav_int());

  // Contrav3<Contrav3<mixed>> rejected: Contrav3<arraykey> is not <: Contrav3<mixed>
  // (would need mixed <: arraykey, which is false)
  VarianceReject::nested_contrav_contrav(contrav3_contrav_mixed());
  VarianceReject::nested_contrav_contrav_simp(contrav3_contrav_mixed());
}

<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

final class Inv2<T> {}
final class Cov2<+T> {}
final class Contrav2<-T> {}

// Factory functions to produce values of specific types
function cov2_int(): Cov2<int> {
  throw new Exception();
}
function contrav2_int(): Contrav2<int> {
  throw new Exception();
}
function contrav2_mixed(): Contrav2<mixed> {
  throw new Exception();
}
function inv2_int(): Inv2<int> {
  throw new Exception();
}
function inv2_arraykey(): Inv2<arraykey> {
  throw new Exception();
}
function cov2_contrav_mixed(): Cov2<Contrav2<mixed>> {
  throw new Exception();
}
function contrav2_contrav_int(): Contrav2<Contrav2<int>> {
  throw new Exception();
}

abstract final class VarianceAccept {

  // Simplified: (Cov2<arraykey>): void
  public static function cov_class_in_contrav_pos<
    T as Cov2<TKey>,
    TKey as arraykey,
  >(T $_): void {}

  public static function cov_class_in_contrav_pos_simp(
    Cov2<arraykey> $_,
  ): void {}

  // Simplified: <T as Cov2<arraykey>>(T): ?T
  public static function cov_class_in_inv_pos<
    T as Cov2<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function cov_class_in_inv_pos_simp<T as Cov2<arraykey>>(
    T $_,
  ): ?T {
    return null;
  }

  // Simplified: (Contrav2<arraykey>): void
  public static function contrav_class_in_contrav_pos<
    T as Contrav2<TKey>,
    TKey super arraykey,
  >(T $_): void {}

  public static function contrav_class_in_contrav_pos_simp(
    Contrav2<arraykey> $_,
  ): void {}

  // Simplified: <T super Contrav2<arraykey>>(T): ?T
  public static function contrav_class_in_inv_pos<
    T super Contrav2<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function contrav_class_in_inv_pos_simp<
    T super Contrav2<arraykey>,
  >(T $_): ?T {
    return null;
  }

  // Simplified: <TKey super arraykey>(Inv2<TKey>): void
  public static function inv_class_in_contrav_pos<
    T as Inv2<TKey>,
    TKey super arraykey,
  >(T $_): void {}

  public static function inv_class_in_contrav_pos_simp<TKey super arraykey>(
    Inv2<TKey> $_,
  ): void {}

  // Simplified: <T super Inv2<TKey>, TKey as arraykey>(T): ?T (unchanged)
  public static function inv_class_in_inv_pos<
    T super Inv2<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function inv_class_in_inv_pos_simp<
    T super Inv2<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  // Simplified: <TKey as arraykey>(Cov2<TKey>): ?TKey
  public static function tkey_in_bound_and_return<
    T as Cov2<TKey>,
    TKey as arraykey,
  >(T $_): ?TKey {
    return null;
  }

  public static function tkey_in_bound_and_return_simp<TKey as arraykey>(
    Cov2<TKey> $_,
  ): ?TKey {
    return null;
  }

  // Simplified: (Cov2<Contrav2<arraykey>>): void
  public static function nested_cov_contrav<
    T as Cov2<Contrav2<TKey>>,
    TKey super arraykey,
  >(T $_): void {}

  public static function nested_cov_contrav_simp(
    Cov2<Contrav2<arraykey>> $_,
  ): void {}

  // Simplified: (Contrav2<Contrav2<arraykey>>): void
  public static function nested_contrav_contrav<
    T as Contrav2<Contrav2<TKey>>,
    TKey as arraykey,
  >(T $_): void {}

  public static function nested_contrav_contrav_simp(
    Contrav2<Contrav2<arraykey>> $_,
  ): void {}
}

function test_accept(): void {
  // Cov2<int> accepted by both (int <: arraykey, Cov2 covariant)
  VarianceAccept::cov_class_in_contrav_pos(cov2_int());
  VarianceAccept::cov_class_in_contrav_pos_simp(cov2_int());

  // Cov2<int> accepted by both (Cov2<int> <: Cov2<arraykey>)
  VarianceAccept::cov_class_in_inv_pos(cov2_int());
  VarianceAccept::cov_class_in_inv_pos_simp(cov2_int());

  // Contrav2<mixed> accepted by both (arraykey <: mixed, contravariance)
  VarianceAccept::contrav_class_in_contrav_pos(contrav2_mixed());
  VarianceAccept::contrav_class_in_contrav_pos_simp(contrav2_mixed());

  // Contrav2<int> accepted by both
  // (Contrav2<arraykey> <: Contrav2<int> since int <: arraykey by contravariance)
  VarianceAccept::contrav_class_in_inv_pos(contrav2_int());
  VarianceAccept::contrav_class_in_inv_pos_simp(contrav2_int());

  // Inv2<arraykey> accepted by both (TKey=arraykey, arraykey super arraykey)
  VarianceAccept::inv_class_in_contrav_pos(inv2_arraykey());
  VarianceAccept::inv_class_in_contrav_pos_simp(inv2_arraykey());

  // Inv2<int> accepted by both (TKey=int, int <: arraykey)
  VarianceAccept::inv_class_in_inv_pos(inv2_int());
  VarianceAccept::inv_class_in_inv_pos_simp(inv2_int());

  // Cov2<int> accepted by both (TKey=int, int <: arraykey)
  VarianceAccept::tkey_in_bound_and_return(cov2_int());
  VarianceAccept::tkey_in_bound_and_return_simp(cov2_int());

  // Cov2<Contrav2<mixed>> accepted by both
  // (Contrav2<mixed> <: Contrav2<arraykey> since arraykey <: mixed)
  VarianceAccept::nested_cov_contrav(cov2_contrav_mixed());
  VarianceAccept::nested_cov_contrav_simp(cov2_contrav_mixed());

  // Contrav2<Contrav2<int>> accepted by both
  // (Contrav2<arraykey> <: Contrav2<int> since int <: arraykey)
  VarianceAccept::nested_contrav_contrav(contrav2_contrav_int());
  VarianceAccept::nested_contrav_contrav_simp(contrav2_contrav_int());
}

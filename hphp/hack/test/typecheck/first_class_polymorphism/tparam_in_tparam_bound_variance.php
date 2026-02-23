<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

final class Inv1<T> {}
final class Cov1<+T> {}
final class Contrav1<-T> {}

abstract final class Variance {
  public static function cov_class_in_cov_pos<
    T super Cov1<TKey>,
    TKey super arraykey,
  >(): ?T {
    return null;
  }

  public static function cov_class_in_contrav_pos<
    T as Cov1<TKey>,
    TKey as arraykey,
  >(T $_): void {}

  public static function cov_class_in_inv_pos<
    T as Cov1<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function contrav_class_in_contrav_pos<
    T as Contrav1<TKey>,
    TKey super arraykey,
  >(T $_): void {}

  public static function contrav_class_in_cov_pos<
    T super Contrav1<TKey>,
    TKey as arraykey,
  >(): ?T {
    return null;
  }

  public static function contrav_class_in_inv_pos<
    T super Contrav1<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function inv_class_in_contrav_pos<
    T as Inv1<TKey>,
    TKey super arraykey,
  >(T $_): void {}

  public static function inv_class_in_cov_pos<
    T super Inv1<TKey>,
    TKey as arraykey,
  >(): ?T {
    return null;
  }

  public static function inv_class_in_inv_pos<
    T super Inv1<TKey>,
    TKey as arraykey,
  >(T $_): ?T {
    return null;
  }

  public static function tkey_in_bound_and_return<
    T as Cov1<TKey>,
    TKey as arraykey,
  >(T $_): ?TKey {
    return null;
  }

  public static function nested_cov_contrav<
    T as Cov1<Contrav1<TKey>>,
    TKey super arraykey,
  >(T $_): void {}

  public static function nested_contrav_contrav<
    T as Contrav1<Contrav1<TKey>>,
    TKey as arraykey,
  >(T $_): void {}
}

function test_type_equality(): void {
  // Both T and TKey eliminated (cov in cov pos)
  hh_expect<HH\FunctionRef<(readonly function(): ?Cov1<arraykey>)>>(
    Variance::cov_class_in_cov_pos<>,
  );

  // Both T and TKey eliminated (cov in contrav pos)
  hh_expect<HH\FunctionRef<(readonly function(Cov1<arraykey>): void)>>(
    Variance::cov_class_in_contrav_pos<>,
  );

  // TKey eliminated, T kept with simplified bound
  hh_expect<HH\FunctionRef<(readonly function<T as Cov1<arraykey>>(T): ?T)>>(
    Variance::cov_class_in_inv_pos<>,
  );

  // Both eliminated (contrav in contrav pos)
  hh_expect<HH\FunctionRef<(readonly function(Contrav1<arraykey>): void)>>(
    Variance::contrav_class_in_contrav_pos<>,
  );

  // Both eliminated (contrav in cov pos)
  hh_expect<HH\FunctionRef<(readonly function(): ?Contrav1<arraykey>)>>(
    Variance::contrav_class_in_cov_pos<>,
  );

  // TKey eliminated, T kept with simplified bound
  hh_expect<
    HH\FunctionRef<(readonly function<T super Contrav1<arraykey>>(T): ?T)>,
  >(Variance::contrav_class_in_inv_pos<>);

  // T eliminated, TKey kept (inv in contrav pos)
  hh_expect<
    HH\FunctionRef<(readonly function<TKey super arraykey>(Inv1<TKey>): void)>,
  >(Variance::inv_class_in_contrav_pos<>);

  // T eliminated, TKey kept (inv in cov pos)
  hh_expect<
    HH\FunctionRef<(readonly function<TKey as arraykey>(): ?Inv1<TKey>)>,
  >(Variance::inv_class_in_cov_pos<>);

  // Neither eliminated (inv in inv pos)
  hh_expect<
    HH\FunctionRef<(readonly function<T super Inv1<TKey>, TKey as arraykey>(
      T,
    ): ?T)>,
  >(Variance::inv_class_in_inv_pos<>);

  // T eliminated, TKey invariant - kept (in both bound and return)
  hh_expect<
    HH\FunctionRef<(readonly function<TKey as arraykey>(Cov1<TKey>): ?TKey)>,
  >(Variance::tkey_in_bound_and_return<>);

  // Both eliminated (nested cov(contrav))
  hh_expect<
    HH\FunctionRef<(readonly function(Cov1<Contrav1<arraykey>>): void)>,
  >(Variance::nested_cov_contrav<>);

  // Both eliminated (nested contrav(contrav))
  hh_expect<
    HH\FunctionRef<(readonly function(Contrav1<Contrav1<arraykey>>): void)>,
  >(Variance::nested_contrav_contrav<>);
}

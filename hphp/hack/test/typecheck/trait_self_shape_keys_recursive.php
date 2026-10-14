<?hh

trait Cycle {
  public function cycle<T1 as ?T2, T2 as T1>(T1 $x): mixed {
    return $x[nameof self];
  }
}

trait T {
  public function shape_bound<
    T1 as ?T2 as shape(nameof T => int),
    T2 as T1,
  >(T1 $x): int {
    return $x[nameof self];
  }
}

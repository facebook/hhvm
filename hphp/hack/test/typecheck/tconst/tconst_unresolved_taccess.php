<?hh
/*
 * This file tests that Tunion type accesses
 * propogate dep_ty info correctly..
 * */
abstract class Zero {}

class ZeroBase<T> {}

abstract class One extends Zero {
  abstract public function genCombinedData<TData, TCombiner as ZeroBase<TData>>(
    TCombiner $combiner,
  ): TData;
}

abstract class ZeroCombiner extends ZeroBase<this::TData> {
  abstract const type TData;
  public function foo(ConstVector<Zero> $targets, One $t): Vector<this::TData> {
    $results = Vector {};
    if (false) {
      $z = $t->genCombinedData($this);
      $results[] = $z;
    }
    $z = $t->genCombinedData($this);
    $results[] = $z;
    return $results;
  }

}

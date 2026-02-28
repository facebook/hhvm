<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {
  public function bar<T>(): void;
}

function dropped1(((dynamic & nonnull) | I) $i1, (dynamic | I) $i2, I $i3): void {
  $i1->bar<int>();
  $i2->bar<int>();
  $i3->bar<int>();
}

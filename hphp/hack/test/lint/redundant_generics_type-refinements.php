<?hh
<<file: __EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type TVal;
}

function in_param<Tak as arraykey>(
  Box with { type TVal = Tak } $_,
): void {}

function in_classname_param<T0 as shape(...)>(
  classname<Box with { type TVal = T0 }> $cls,
  T0 $s,
): void {}

function returns_num_box_eq_good<T1 as num>(
): Box with { type TVal = T1 } {
  while (true) {}
}

<?hh

interface I1 {}

interface I2 {}

function f_no_bounds<T>(HH\EnumClass\Label<mixed, T> $label, T $value): void {}

function f_single_bound<T as arraykey>(
  HH\EnumClass\Label<mixed, T> $label,
  T $value,
): void {}

function f_multi_bounds_simplify<T as ?arraykey as nonnull>(
  HH\EnumClass\Label<mixed, T> $label,
  T $value,
): void {}

function f_multi_bounds_dont_simplify<T as I1 as I2>(
  HH\EnumClass\Label<mixed, T> $label,
  T $value,
): void {}

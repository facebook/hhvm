<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class TransitivityC {}

function stale_order<T super class<TransitivityC>, TU>(
  T $value,
): (T, TU) where T as TU, TU as class<TransitivityC>, T as string {
  return tuple($value, $value);
}

function fresh_order<T super class<TransitivityC>, TU>(
  T $value,
): (T, TU) where T as string, T as TU, TU as class<TransitivityC> {
  return tuple($value, $value);
}

function get_transitivity_class_pointer(): class<TransitivityC> {
  return TransitivityC::class;
}

function test_transitivity(): void {
  hh_show(stale_order(get_transitivity_class_pointer()));
  hh_show(fresh_order(get_transitivity_class_pointer()));
}

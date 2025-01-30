<?hh
<<file: __EnableUnstableFeatures('open_tuples')>>

interface I {}
abstract class C implements I {
  const type TC = (int, string);
  const type TC2 = (bool, float, optional int);
  const type TC3 = (float, nonnull...);

}
<<__EntryPoint>>
function main(): void {
  $x = type_structure(C::class, 'TC');
  $y = type_structure(C::class, 'TC2');
  $z = type_structure(C::class, 'TC3');
  hh_show($x);
  hh_show($x['elem_types']);
  // Expect an error, because TC doesn't have optional elements
  $x['optional_elem_types'];
  // Expect an error, because TC doesn't have a variadic element
  $x['variadic_type'];
  hh_show($y);
  hh_show($y['elem_types']);
  hh_show($y['optional_elem_types']);
  // Expect an error, because TC2 doesn't have a variadic element
  $y['variadic_type'];
  hh_show($z);
  hh_show($z['elem_types']);
  // Expect an error, because TC3 doesn't have optional elements
  $z['optional_elem_types'];
  hh_show($z['variadic_type']);
}

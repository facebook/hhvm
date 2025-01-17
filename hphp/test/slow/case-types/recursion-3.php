<?hh

<<file:__EnableUnstableFeatures('case_types')>>

case type C1 = int | shape("foo" => C1);

case type C2 = string | shape("foo" => C3);
case type C3 = int | shape("foo" => C2);

case type C4 = string | shape("foo" => C5, "bar" => C5);
case type C5 = int | shape("foo" => C6);
case type C6 = int | float;

type T1 = shape("foo" => T2);
type T2 = shape("foo" => T1);

case type C7 = string | shape("foo" => T3);
type T3 = T4;
type T4 = C7;

<<__EntryPoint>>
function main() :mixed{
  printf("C1: %s\n", var_export(type_structure_for_alias('C1'), true));
  printf("-----\n");
  printf("C2: %s\n", var_export(type_structure_for_alias('C2'), true));
  printf("-----\n");
  printf("C7: %s\n", var_export(type_structure_for_alias('C7'), true));
}

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
  printf("5 is C1: %d\n", 5 is C1);
  printf("'foo' is C1: %d\n", 'foo' is C1);
  printf("shape('foo' => 5) is C1: %d\n", shape('foo' => 5) is C1);
  printf("shape('foo' => 'abc') is C1: %d\n", shape('foo' => 'abc') is C1);
  printf("shape('foo' => shape('foo' => 5)) is C1: %d\n", shape('foo' => shape('foo' => 5)) is C1);
  printf("shape('foo' => shape('foo' => 'abc')) is C1: %d\n", shape('foo' => shape('foo' => 'abc')) is C1);
  printf("-----\n");
  printf("C2: %s\n", var_export(type_structure_for_alias('C2'), true));
  printf("5 is C2: %d\n", 5 is C2);
  printf("'foo' is C2: %d\n", 'foo' is C2);
  printf("shape('foo' => 5) is C2: %d\n", shape('foo' => 5) is C2);
  printf("shape('foo' => 'abc') is C2: %d\n", shape('foo' => 'abc') is C2);
  printf("shape('foo' => shape('foo' => 5)) is C2: %d\n", shape('foo' => shape('foo' => 5)) is C2);
  printf("shape('foo' => shape('foo' => 'abc')) is C2: %d\n", shape('foo' => shape('foo' => 'abc')) is C2);
  printf("-----\n");
  printf("C7: %s\n", var_export(type_structure_for_alias('C7'), true));
}

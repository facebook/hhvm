<?hh

class A {}
type T = A;
const C = 10;

<<__EntryPoint>>
function main(): void {
  var_dump(HH\autoload_type_to_path(
    A::class,
  ));
  var_dump(HH\autoload_type_to_path(
    'T',
  ));
  var_dump(HH\autoload_type_alias_to_path(
    A::class,
  ));
  var_dump(HH\autoload_type_alias_to_path(
    'T',
  ));
  var_dump(HH\autoload_type_or_type_alias_to_path(
    A::class,
  ));
  var_dump(HH\autoload_type_or_type_alias_to_path(
    'T',
  ));
  var_dump(HH\autoload_type_to_path(
    'notexisting',
  ));
  var_dump(HH\autoload_function_to_path(
    'main',
  ));
  var_dump(HH\autoload_function_to_path(
    'notexisting',
  ));
  var_dump(HH\autoload_constant_to_path(
    'C',
  ));
  var_dump(HH\autoload_constant_to_path(
    'notexisting',
  ));
}

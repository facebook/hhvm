<?hh

class A {}

const int C = 1;

newtype T = string;

<<__EntryPoint>>
function main(): void {
  var_dump(HH\autoload_path_to_types(
    __DIR__.'/autoload_path_to_symbols.php',
  ));
  var_dump(HH\autoload_path_to_types(
    __DIR__.'/autoload_path_to_symbols.php.notexists',
  ));
  var_dump(HH\autoload_path_to_functions(
    __DIR__.'/autoload_path_to_symbols.php',
  ));
  var_dump(HH\autoload_path_to_functions(
    __DIR__.'/autoload_path_to_symbols.php.notexists',
  ));
  var_dump(HH\autoload_path_to_constants(
    __DIR__.'/autoload_path_to_symbols.php',
  ));
  var_dump(HH\autoload_path_to_constants(
    __DIR__.'/autoload_path_to_symbols.php.notexists',
  ));
  var_dump(HH\autoload_path_to_type_aliases(
    __DIR__.'/autoload_path_to_symbols.php',
  ));
  var_dump(HH\autoload_path_to_type_aliases(
    __DIR__.'/autoload_path_to_symbols.php.notexists',
  ));
}

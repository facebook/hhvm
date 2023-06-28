<?hh

function dump(vec<string> $var) :mixed{
  sort(inout $var);
  var_dump($var);
}

<<__EntryPoint>>
function main(): void {
  dump(HH\autoload_path_to_types(
    __DIR__.'/autoload.inc',
  ));
  dump(HH\autoload_path_to_types(
    __DIR__.'/autoload.inc.notexists',
  ));
  dump(HH\autoload_path_to_functions(
    __DIR__.'/autoload.inc',
  ));
  dump(HH\autoload_path_to_functions(
    __DIR__.'/autoload.inc.notexists',
  ));
  dump(HH\autoload_path_to_constants(
    __DIR__.'/autoload.inc',
  ));
  dump(HH\autoload_path_to_constants(
    __DIR__.'/autoload.inc.notexists',
  ));
  dump(HH\autoload_path_to_type_aliases(
    __DIR__.'/autoload.inc',
  ));
  dump(HH\autoload_path_to_type_aliases(
    __DIR__.'/autoload.inc.notexists',
  ));
}

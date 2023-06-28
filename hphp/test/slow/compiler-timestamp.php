<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(HHVM_COMPILER_TIMESTAMP);
  var_dump(ini_get('hphp.compiler_timestamp'));
}

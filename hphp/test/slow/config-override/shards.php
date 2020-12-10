<?hh

<<__EntryPoint>>
function main() {
  var_dump(ini_get('hhvm.array_provenance'));
  var_dump(ini_get('hhvm.log_array_provenance'));
}

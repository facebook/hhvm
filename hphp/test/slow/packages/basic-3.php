<?hh

module a;

<<__EntryPoint>>
function main() :mixed{
  var_dump(ini_get('hhvm.active_deployment'));
  var_dump(package_exists('prod'));
  var_dump(package_exists('intern'));
  var_dump(package_exists('baz'));
}

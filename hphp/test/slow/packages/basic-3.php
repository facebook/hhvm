<?hh

module a;

<<__EntryPoint>>
function main() :mixed{
  var_dump(ini_get('hhvm.active_deployment'));
  var_dump(package_exists('foo'));
  var_dump(package_exists('bar'));
  var_dump(package_exists('baz'));
  var_dump(package_exists('foobar')); // not real
}

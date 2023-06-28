<?hh

<<__EntryPoint>>
function main() :mixed{
  include "module.inc";
  var_dump(module_exists('a')); // true
  var_dump(module_exists('b')); // false
  var_dump(module_exists('foo', false)); // true
  var_dump(module_exists('boo', false)); // false
}

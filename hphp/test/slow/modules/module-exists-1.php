<?hh

<<__EntryPoint>>
function main() :mixed{
  include "module.inc";
  var_dump(\HH\module_exists('a')); // true
  var_dump(\HH\module_exists('b')); // false
  var_dump(\HH\module_exists('foo', false)); // true
  var_dump(\HH\module_exists('boo', false)); // false
}

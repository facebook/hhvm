<?hh
<<file:__EnableUnstableFeatures('package')>>

<<__EntryPoint>>
function main() :mixed{
  var_dump(ini_get('hhvm.active_deployment'));
  var_dump(package foo);
  var_dump(package bar);
  var_dump(package baz);
  var_dump(package foobar); // not real
}

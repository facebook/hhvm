<?php

class C {}
function foo() { return mt_rand() ? array(new C) : array(new C, new C); }
function val() {
  $x = '0';
  apc_store('asd', $x);
  return apc_fetch('asd');
}
function bar() {
  $x = foo();
  return $x[\HH\array_key_cast(val())];
}
function main() {
  var_dump(bar());
}

<<__EntryPoint>>
function main_array_046() {
main();
}

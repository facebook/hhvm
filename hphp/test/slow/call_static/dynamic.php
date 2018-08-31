<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static function __callStatic($name, $arguments) {
    echo "__callStatic ==> $name\n";
  }
}

function call($meth) {
  $s = 'A::' . $meth;
  $s();
}

function test() {
  call('foo');
  call('bar');
  call('baz');
}

<<__EntryPoint>>
function main_dynamic() {
test();
test();
}

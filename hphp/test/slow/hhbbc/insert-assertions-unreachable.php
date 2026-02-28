<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref { public $v = 0; }
function byref(inout $x) :mixed{ $x->v = 42; }
function build() :mixed{
  $d = dict[];
  $k = __hhvm_intrinsics\launder_value('key');
  while (__hhvm_intrinsics\launder_value(false)) {
    $d[$k] ??= new Ref();
    byref(inout $d);
  }
  return $d;
}


<<__EntryPoint>>
function main_insert_assertions_unreachable() :mixed{
var_dump(build());
}

<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function foo(varray $data) :mixed{
  $ret = array_shift(inout $data);
  if ($ret === 'false') {
    $ret = false;
  } else {
    $ret = (bool)$ret;
  }
  return $ret;
}
<<__EntryPoint>> function main(): void {
var_dump(foo(vec[]));
}

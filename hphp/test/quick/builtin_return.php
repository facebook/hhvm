<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function foo(array $data) {
  $ret = current($data);
  if ($ret === 'false') {
    $ret = false;
  } else {
    $ret = (bool)$ret;
  }
  return $ret;
}
<<__EntryPoint>> function main(): void {
var_dump(foo(varray[]));
}

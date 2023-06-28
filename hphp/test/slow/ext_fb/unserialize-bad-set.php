<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  $ret = null;
  var_dump(fb_unserialize("\x14\x02\x01\x14\x02\x01\x02\x01\x01\x01", inout $ret));
  var_dump($ret);
}

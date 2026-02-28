<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  $success = null;
  var_dump(
    fb_unserialize("\xf9\xf0\xff\x00\x00\x00\x00\x00\x00\x01a", inout $success)
  );
  var_dump($success);
}

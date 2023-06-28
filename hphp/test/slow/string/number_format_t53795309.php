<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  $bin_repr = "\x00\x00\x00\x00\x00\x00\x00\x80";
  $double_num = unpack("dnum", $bin_repr)['num'];
  var_dump(number_format($double_num, 100));
}

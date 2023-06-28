<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


<<__EntryPoint>>
function ref_unserialize() :mixed{
  echo "========== ref_unserialize =================================\n";
  $ref_str = "a:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  unserialize($ref_str);
}

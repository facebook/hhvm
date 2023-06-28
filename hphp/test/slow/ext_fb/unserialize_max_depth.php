<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function make_fb_serialize_struct($size) :mixed{
  return str_repeat("\x0a\x02\x01", $size) . "\x02\x01" . str_repeat("\x01", $size);
}

function make_fb_serialize_vector($size) :mixed{
  return str_repeat("\x12", $size) . str_repeat("\x01", $size);
}

function make_fb_serialize_list($size) :mixed{
  return str_repeat("\x13\x02\x01", $size) . "\x02\x01" . str_repeat("\x01", $size);
}

function make_fb_cs_list_map($size) :mixed{
  return str_repeat("\xfa\xfd", $size) . str_repeat("\xfc", $size);
}

function make_fb_cs_map($size) :mixed{
  return str_repeat("\xfb\x01", $size) . "\x01" . str_repeat("\xfc", $size);
}

function make_fb_cs_vector($size) :mixed{
  return str_repeat("\xfe", $size) . str_repeat("\xfc", $size);
}

function test($serialized) :mixed{
  $ret = null;
  var_dump(
    fb_unserialize(
      $serialized,
      inout $ret,
      FB_SERIALIZE_HACK_ARRAYS
    )
  );
  var_dump($ret);
}

function tests($size) :mixed{
  test(make_fb_serialize_struct($size));
  test(make_fb_serialize_vector($size));
  test(make_fb_serialize_list($size));
  test(make_fb_cs_list_map($size));
  test(make_fb_cs_map($size));
  test(make_fb_cs_vector($size));
}

<<__EntryPoint>>
function main() :mixed{
  tests(10);
  tests(1026);
}

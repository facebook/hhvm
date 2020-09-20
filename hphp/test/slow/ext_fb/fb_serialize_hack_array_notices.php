<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main(): void {
  // default
  fb_serialize(varray[]);
  fb_serialize(darray[]);
  try {
    fb_serialize(vec[]);
    echo "fail\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(dict[]);
    echo "fail\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(keyset[]);
    echo "fail\n";
  } catch (Exception $e) {}

  // default
  fb_serialize(varray[], FB_SERIALIZE_VARRAY_DARRAY);
  fb_serialize(darray[], FB_SERIALIZE_VARRAY_DARRAY);
  try {
    fb_serialize(vec[], FB_SERIALIZE_VARRAY_DARRAY);
    echo "fail\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(dict[], FB_SERIALIZE_VARRAY_DARRAY);
    echo "fail\n";
  } catch (Exception $e) {}
  try {
    fb_serialize(keyset[], FB_SERIALIZE_VARRAY_DARRAY);
    echo "fail\n";
  } catch (Exception $e) {}

  fb_serialize(varray[], FB_SERIALIZE_HACK_ARRAYS);
  fb_serialize(darray[], FB_SERIALIZE_HACK_ARRAYS);
  fb_serialize(vec[], FB_SERIALIZE_HACK_ARRAYS);
  fb_serialize(dict[], FB_SERIALIZE_HACK_ARRAYS);
  try {
    fb_serialize(keyset[], FB_SERIALIZE_HACK_ARRAYS);
    echo "fail\n";
  } catch (Exception $e) {}

  fb_serialize(varray[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(darray[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(vec[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(dict[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
  fb_serialize(keyset[], FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS);
}
